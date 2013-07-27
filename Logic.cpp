#include <cstdlib>
#include <cstring>
#include "Patterns.h"
#include "Logic.h"

Logic::Logic() : player_(1), generation_(0), nextGenerationTime_(0), net_(31338) {
}

Logic::~Logic() {
}

void Logic::reset(vec2i size) {
  field_.reset(size);
  field_.mark(vec2i(17), 23, 1);
  field_.mark(size-vec2i(17), 23, 2);

  player_ = 1;
  generation_ = 0;
  nextGenerationTime_ = 0;
  memset(players_, 0, sizeof(players_));
}

void Logic::update(u32 now_ms) {
  /// \todo update net
  comcenter_.update();
  while (nextGenerationTime_ < now_ms) {

    // update player commands
    comcenter_.nextGeneration(/*net_*/);
    const u32 n_commands = comcenter_.getCommandCount();
    const Command *commands = comcenter_.getCommands();
    for (u32 i = 0; i < n_commands; ++i)
      processCommand(commands[i]);
    
    // update game field
    field_.calcNextGeneration();

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
    ++generation_;
  }
}

void Logic::place(vec2i pos, Rotation rotation, u32 pattern_index) {
  Command cmd;
  cmd.player = player_;
  cmd.type = Command::Place;
  cmd.place.position = pos;
  cmd.place.rotation = rotation;
  cmd.place.pattern = pattern_index;
  comcenter_.sendCommand(cmd);
}

void Logic::processCommand(const Command &command) {
  switch(command.type) {
    case Command::Place:
      cmdPlace(command.player, command.place.position,
        command.place.rotation, command.place.pattern);
      break;
    default:
      break;
  }
}

void Logic::cmdPlace(u32 player, vec2i pos, u32 rotation, u32 ipat) {
  Player &plr = players_[player];
  const Pattern &pat = g_patterns[ipat];

  L("player %d puts pattern %d at pos (%d, %d)", player, ipat, pos.x, pos.y);
  
  if (plr.resources < pat.cost) return;

  if (field_.place(pos, ::RotationNone, player, vec2i(pat.width, pat.height), pat.map))
    plr.resources -= pat.cost;
}

