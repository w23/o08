#include <cstdlib>
#include <cstring>
#include "Patterns.h"
#include "Logic.h"

Logic::Logic(vec2i size)
  : size_(size), frame_(size_.x * size_.y), current_(0)
  , cells_(new Cell[frame_ * 2]), generation_(0)
  , nextGenerationTime_(0)
  , player_(1) {
  memset(cells_, 0, frame_ * 2 * sizeof(Cell));

  // fill with randomness
  Cell *p = cells_ + current_ * frame_ + size.x + 1;
  for (int y = 2; y < size_.y; ++y, p += 2)
    for (int x = 2; x < size_.x; ++x, ++p) {
      //p->setAlive(rand() & 1);
      //if (p->isAlive())
      //  p->setOwner((rand() & Cell::OwnerMask) % 2 + 1);
      p->setAlive((rand() & 7) == 1);
      p->setOwner( (x>size_.x/2) ? 1 : 2 );
    }
}

Logic::~Logic() {
  delete[] cells_;
}

void Logic::update(u32 now_ms) {
  comcenter_.update();
  while (nextGenerationTime_ < now_ms) {
    comcenter_.nextGeneration();
    const u32 n_commands = comcenter_.getCommandCount();
    const Command *commands = comcenter_.getCommands();
    for (u32 i = 0; i < n_commands; ++i)
      processCommand(commands[i]);
    step();
    nextGenerationTime_ += PLANCK_TIME;
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
  const Pattern &pat = g_patterns[ipat];
  if (players_[player].resources < pat.cost) return;

  vec2i psize(pat.width, pat.height);

  int x_advance, y_advance;
  switch (rotation) {
    case RotationNone:
      x_advance = 1;
      y_advance = size_.x - psize.x;
      break;
    case Rotation90:
      x_advance = 1;
      y_advance = size_.x - psize.x;
      break;
    case Rotation180:
      x_advance = 1;
      y_advance = size_.x - psize.x;
      break;
    case Rotation270:
      x_advance = 1;
      y_advance = size_.x - psize.x;
      break;
    default:
      return;
  }

  pos -= vec2i((psize + vec2i(1)) / 2);

  if (pos.x < 0 || pos.y < 0 ||
    pos.x > size_.x - pat.width ||
    pos.y > size_.y - pat.height)
    return;

  Cell *p = cells_ + current_ * frame_ + pos.x + size_.x * pos.y;
  for (int y = 0; y < psize.y; ++y, p += y_advance)
    for (int x = 0; x < psize.x; ++x, p += x_advance)
      if (p->getOwner() != player || p->isAlive()) return;


  const u8 *map = pat.map; /// \todo rotations
  p = cells_ + current_ * frame_ + pos.x + size_.x * pos.y;
  for (int y = 0; y < psize.y; ++y, p += y_advance)
    for (int x = 0; x < psize.x; ++x, p += x_advance, ++map)
      p->setAlive(*map != 0);
}

void Logic::step() {
  // zero player stuff
  for (int i = 0; i < MAX_PLAYERS; ++i) players_[i].area = players_[i].cells = 0;

  u32 next = 1 & (current_ + 1);
  const Cell *in = cells_ + current_ * frame_ + size_.x + 1;
  Cell *out = cells_ + next * frame_ + size_.x + 1;
  for (int y = 2; y < size_.y; ++y, in += 2, out += 2)
    for (int x = 2; x < size_.x; ++x, ++in, ++out) {
      u32 new_owner = 0;
      u32 neighbors = 0;
      u32 histo[MAX_PLAYERS];
      memset(histo, 0, 4 * MAX_PLAYERS);
#define CHECK_CELL(c) \
        { \
          u32 cstate = in[c].state; \
          if (cstate & Cell::AliveMask) { \
            ++neighbors;\
            u32 player = cstate & Cell::OwnerMask; \
            if (histo[player] == 1) new_owner = player; \
            ++histo[player]; \
          } \
        }
      CHECK_CELL(-1-size_.x);
      CHECK_CELL(  -size_.x);
      CHECK_CELL( 1-size_.x)
      CHECK_CELL(-1);
      CHECK_CELL( 1);
      CHECK_CELL(-1+size_.x);
      CHECK_CELL(   size_.x);
      CHECK_CELL( 1+size_.x);

      *out = *in;
      if (neighbors == 2) continue;
      if (neighbors == 3) {
        if (!out->isAlive()) {
          out->setAlive(true);
          if (new_owner != 0) out->setOwner(new_owner);
        }
      } else out->setAlive(false);

      Player &player = players_[out->getOwner()];
      ++player.area;
      if (out->isAlive()) ++player.cells;
      else if (in->isAlive()) ++player.resources; // cell death is net gain
    }
    current_ = next;
    ++generation_;
}

