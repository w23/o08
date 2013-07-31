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
  int r2 = radius * radius;
  rect2i rect(pos - vec2i(radius), pos + vec2i(radius));
  rect.clip(rect2i(vec2i(1), size_-vec2i(1)));
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
  vec2i origin, advance;
  switch (rotation) {
    case Rotation0:
      origin = vec2i(0);
      advance = vec2i(1, 0);
      break;
    case Rotation90:
      origin = vec2i(0, size.y - 1);
      advance = vec2i( - size.x, size.y * size.x + 1);
      size = size.yx();
      break;
    case Rotation180:
      origin = vec2i(size.x - 1, size.y - 1);
      advance = vec2i(-1, 0);
      break;
    case Rotation270:
      origin = vec2i(size.x - 1, 0);
      advance = vec2i(size.x, - size.x * size.y - 1);
      size = size.yx();
      break;
    default:
      return false;
  }

  pos -= vec2i(size / 2);

  if (pos.x < 0 || pos.y < 0 ||
    pos.x > size_.x - size.x ||
    pos.y > size_.y - size.y)
    return false;

  u32 stride = size_.x - size.x;
  Cell *p = getCells() + pos.x + size_.x * pos.y;
  for (int y = 0; y < size.y; ++y, p += stride)
    for (int x = 0; x < size.x; ++x, ++p)
      if (p->getOwner() != player || p->isAlive()) return false;

  map += origin.x + origin.y * size.x;
  p = getCells() + pos.x + size_.x * pos.y;
  for (int y = 0; y < size.y; ++y, p += stride, map += advance.y)
    for (int x = 0; x < size.x; ++x, ++p, map += advance.x)
      p->setAlive(*map != 0);
  return true;
}

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

