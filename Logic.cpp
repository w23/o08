#include <cstdlib>
#include <cstring>
#include "Patterns.h"
#include "Logic.h"

static const u32 c_version = 1;

struct Packet {
  enum PacketCode : u32 {
    Connect,
    GameInfo,
    Play,
    Pause,
    Sync,
    End
  } code;
  union {
    struct {
      vec2i size;
      u32 player;
    } game_info;
    struct {
      u32 generation, player;
      u32 n_commands;
      Command commands[MAX_PLAYER_COMMANDS];
    } sync;
  };

  Packet(PacketCode _code) : code(_code) {}
};


Logic::Logic() : state_(Idle) {}

Logic::~Logic() {}

void Logic::reset(vec2i size, u32 player) {
  L("Resetting game to field %dx%d player %d", size.x, size.y, player);
  field_.reset(size);
  field_.mark(vec2i(17), 23, 1);
  field_.mark(size-vec2i(17), 23, 2);

  player_ = player;
  comcenter_.set_active_players(2, player_);
  nextGenerationTime_ = 0;
  memset(players_, 0, sizeof(players_));
  comcenter_.reset();
}

void Logic::create(vec2i size, int listen_port) {
  reset(size, 1);
  net_.listen(listen_port);
  state_ = Listening;
}

void Logic::connect(const char *remote_host, int remote_port) {
  net_.connect(remote_host, remote_port);
  Packet out(Packet::Connect);
  net_.write(&out, sizeof(Packet));
  state_ = Connecting;
}

void Logic::update(u32 now_ms) {
  update_network();

  if (state_ != Playing) return;

  while (nextGenerationTime_ < now_ms) {
    if (!comcenter_.can_advance()) {
      //L("LOGIC WARNING: You can NOT advance.");
      net_.send();
      break;
    }

    // send sync
    {
      Packet out(Packet::Sync);
      u32 n_commands;
      const Command *commands = comcenter_.get_new_commands(player_, n_commands);
      out.sync.generation = comcenter_.new_generation();
      out.sync.player = player_;
      out.sync.n_commands = n_commands;
      memcpy(out.sync.commands, commands, sizeof(Command) * n_commands);

      // do not go further if send failed
      if (!net_.write(&out, sizeof(Packet))) break;

      //L("LOGIC INFO: send sync gen %d player %d commands %d",
      //  comcenter_.new_generation(), player_, n_commands);
    }

    comcenter_.advance();

    // update game field
    field_.calcNextGeneration();

    // update player commands
    for (u32 i = 0;; ++i) {
      const CommandEx *cmd = comcenter_.get_current_generation_command(i);
      if (!cmd) break;
      processCommand(*cmd);
    }

    // update economy
    for (u32 i = 0; i < MAX_PLAYERS; ++i) {
      Player &p = players_[i];
      const Field::Player &fp = field_.getPlayer(i);
      p.area = fp.area;
      p.cells = fp.cells;
      p.resources += fp.deaths * 2 - fp.births; // lols
    }

    // update state
    nextGenerationTime_ += PLANCK_TIME;
  }
}

void Logic::place(vec2i pos, Rotation rotation, u32 pattern_index) {
  Command *cmd = comcenter_.get_new_command_slot(player_);
  if (!cmd) {
    L("WARNING: No place for new local player command");
    return;
  }

  cmd->code = Command::Place;
  cmd->place.position = pos;
  cmd->place.rotation = rotation;
  cmd->place.pattern = pattern_index;
}

void Logic::update_network() {
  for (;;) {
    u32 size;
    const Packet *packet = reinterpret_cast<const Packet*>(net_.read(size));
    if (!packet) break;
    if (size != sizeof(Packet)) {
      L("LOGIC WARNING: packet size %d != %d", size, sizeof(Packet));
    }

    switch(packet->code) {
    case Packet::Connect:
      L("LOGIC: PKT CONN");
      if (state_ == Listening) {
        L("LOGIC SEND: GAMEINFO");
        Packet out(Packet::GameInfo);
        out.game_info.size = field_.getSize();
        out.game_info.player = 2;
        if (net_.write(&out, sizeof(out))) state_ = Playing;
      }
      break;

    case Packet::GameInfo:
      L("LOGIC: PKT GAMEINFO");
      if (state_ == Connecting) {
        reset(packet->game_info.size, packet->game_info.player);
        state_ = Playing;
      }
      break;

    case Packet::Sync:
      //L("LOGIC: PKT SYNC");
      if (state_ == Playing) {
        //L("LOGIC: PKT SYNC gen %d player %d",
        //  packet->sync.generation, packet->sync.player);
        //L("LOGIC: PKT SYNC player %d commands %d",
        //  packet->sync.player, packet->sync.n_commands);
        comcenter_.write_remote_commands(
          packet->sync.player, packet->sync.generation,
          packet->sync.n_commands, packet->sync.commands);
      }
      break;

    case Packet::Play:
    case Packet::Pause:
    case Packet::End:
    default:
      L("Unknown packet with code %d", packet->code);
    }
  }
}

void Logic::processCommand(const CommandEx &command) {
  switch(command.cmd.code) {
    case Command::Place:
      cmdPlace(command.player, command.cmd.place.position,
        command.cmd.place.rotation, command.cmd.place.pattern);
      break;
    default:
      break;
  }
}

void Logic::cmdPlace(u32 player, vec2i pos, u32 rotation, u32 ipat) {
  Player &plr = players_[player];
  const Pattern &pat = g_patterns[ipat];

  //L("player %d puts pattern %d at pos (%d, %d)", player, ipat, pos.x, pos.y);

  if (plr.resources < pat.cost) return;

  if (field_.place(pos, static_cast<Rotation>(rotation), player,
    vec2i(pat.width, pat.height), pat.map))
    plr.resources -= pat.cost;
}

