#include <cstdlib>
#include <cstring>
#include "Patterns.h"
#include "Logic.h"

static const u32 c_version = 1;

enum class PacketCode : u32 {
  Connect = 1,
  GameInfo,
  Play,
  Pause,
  CommandPlace,
  End
};

struct PacketGameInfo {
  vec2i size;
  u32 player;
};

Logic::Logic() : state_(Idle) {}

Logic::~Logic() {}

void Logic::reset(vec2i size, u32 player) {
  comcenter_.reset();
  field_.reset(size);
  field_.mark(vec2i(17), 23, 1);
  field_.mark(size-vec2i(17), 23, 2);

  player_ = player;
  nextGenerationTime_ = 0;
  memset(players_, 0, sizeof(players_));
}

void Logic::create(vec2i size, int listen_port) {
  reset(size, 1);
  net_.listen(listen_port);
  state_ = Listening;
}

void Logic::connect(const char *remote_host, int remote_port) {
  net_.connect(remote_host, remote_port);
  net_.enqueue(&c_version, 4, static_cast<u32>(PacketCode::Connect));
  state_ = Connecting;
}

void Logic::update(u32 now_ms) {
  update_network();
  if (state_ != Playing) return;

  while (nextGenerationTime_ < now_ms) {
    if (!comcenter_.next_generation()) {
      L("WARNING: You can NOT advance.");
      break;
    }

    // update game field
    field_.calcNextGeneration();

    // send local commands upstream
    for (u32 i = 0;; ++i) {
      const Command *cmd = comcenter_.get_due_generation_command(i);
      if (!cmd) break;
      if (net_.enqueue_get_free() < 1) {
        L("FATAL: not enough space in output queue");
        break;
      }

      net_.enqueue(&cmd->payload, sizeof(cmd->payload), cmd->code);
    }

    net_.send();

    // update player commands
    for (u32 i = 0;; ++i) {
      const Command *cmd = comcenter_.get_current_generation_command(i);
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
  Command *cmd = comcenter_.get_local_command_slot();
  if (!cmd) {
    L("WARNING: No place for net command");
    return;
  }

  cmd->code = static_cast<u32>(PacketCode::CommandPlace);
  cmd->payload.player = player_;
  cmd->payload.place.position = pos;
  cmd->payload.place.rotation = rotation;
  cmd->payload.place.pattern = pattern_index;
}

void Logic::update_network() {
  u8 payload[MAX_NET_PAYLOAD];
  for (;;) {
    u32 code = net_.receive(payload);
    if (code == 0) break;

    switch(static_cast<PacketCode>(code)) {
    case PacketCode::Connect:
      if (state_ == Listening) {
        PacketGameInfo infopkt = {field_.getSize(), 2};
        KP_ASSERT(net_.enqueue_get_free() > 1);
        net_.enqueue(&infopkt, sizeof(infopkt), static_cast<u32>(PacketCode::GameInfo));
        state_ = Playing;
      }
      break;

    case PacketCode::GameInfo:
      if (state_ == Connecting) {
        const PacketGameInfo *infopkt = reinterpret_cast<const PacketGameInfo*>(payload);
        reset(infopkt->size, infopkt->player);
        state_ = Playing;
      }
      break;

    case PacketCode::CommandPlace:
      if (state_ == Playing) {
        u32 generation = *reinterpret_cast<u32*>(payload);
        Command *cmd = comcenter_.get_remote_command_slot(generation);
        if (cmd) {
          cmd->code = code;
          memcpy(&cmd->payload, payload, sizeof(cmd->payload));
        }
      }
      break;

    case PacketCode::Play:
    case PacketCode::Pause:
    case PacketCode::End:
    default:
      L("Unknown packet with code %d", code);
    }
  }
}

void Logic::processCommand(const Command &command) {
  switch(command.code) {
    case static_cast<u32>(PacketCode::CommandPlace):
      cmdPlace(command.payload.player, command.payload.place.position,
        command.payload.place.rotation, command.payload.place.pattern);
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

