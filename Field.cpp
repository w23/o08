#include <cstring>
#include "Patterns.h"
#include "Field.h"

Field::Field() : size_(0), frame_(0), current_(0), cells_(nullptr) {}

void Field::reset(vec2i size) {
  if (size_ != size) {
    size_ = size;
    frame_ = size_.x * size_.y;
    if (cells_) delete[] cells_;
    cells_ = new Cell[frame_ * 2];
  }
  memset(cells_, 0, frame_ * 2 * sizeof(Cell));
  current_ = 0;
  memset(players_, 0, sizeof(players_));
}

Field::~Field() {
  delete[] cells_;
}

void Field::mark(vec2i pos, u32 radius, u32 player) {
  u32 r2 = radius * radius;
  rect2i rect(pos - vec2i(radius), pos + vec2i(radius));
  rect.clip(rect2i(vec2i(1), size_-vec2i(1)));
  L("%d %d %d %d", rect.min.x, rect.min.y, rect.max.x, rect.max.y);
  Cell *p = getCells() + rect.min.x + rect.min.y * size_.x;
  int stride = size_.x - rect.width();
  for (int y = rect.min.y; y < rect.max.y; ++y, p += stride)
    for (int x = rect.min.x; x < rect.max.x; ++x, ++p) 
      if ((vec2i(x,y) - pos).length_sq() < r2){
        p->setAlive(true); ///< \todo life-fill modes: 0, 1, preserve, random, ...
        p->setOwner(player);
      }
}

bool Field::place(vec2i pos, Rotation rotation, u32 player, vec2i size, const u8 *map) {
  return false;
}
#if 0
void Field::cmdPlace(u32 player, vec2i pos, u32 rotation, u32 ipat) {
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
#endif

void Field::calcNextGeneration() {
  // zero player stuff
  memset(players_, 0, sizeof(players_));

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
          ++players_[new_owner].births;
        }
      } else out->setAlive(false);

      Player &player = players_[out->getOwner()];
      ++player.area;
      if (out->isAlive()) ++player.cells;
      else if (in->isAlive()) ++player.deaths;
    }
    current_ = next;
}

