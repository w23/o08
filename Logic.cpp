#include <cstdlib>
#include <cstring>
#include "Logic.h"

Logic::Logic(vec2i size) : size_(size), frame_(size_.x * size_.y), current_(0)
  , cells_(new Cell[frame_ * 2]) {
  memset(cells_, 0, frame_ * 2 * sizeof(Cell));

  // fill with randomness
  Cell *p = cells_ + current_ * frame_ + size.x + 1;
  for (int y = 2; y < size_.y; ++y, p += 2)
    for (int x = 2; x < size_.x; ++x, ++p) {
      p->setAlive(rand() & 1);
      p->setOwner(rand() & Cell::PlayerMask);
    }
}

Logic::~Logic() {
  delete[] cells_;
}

void Logic::step() {
  u32 next = 1 & (current_ + 1);
  const Cell *in = cells_ + current_ * frame_ + size_.x + 1;
  Cell *out = cells_ + next * frame_ + size_.x + 1;
  for (int y = 2; y < size_.y; ++y, in += 2, out += 2)
    for (int x = 2; x < size_.x; ++x, ++in, ++out) {
      u32 new_owner = 0;
      int neighbors = 0;
      int histo[Cell::PlayerMask+1];
      memset(histo, 0, 4 * (Cell::PlayerMask+1));
#define CHECK_CELL(c) \
        { \
          u32 cstate = in[c].state; \
          if (cstate & Cell::AliveMask) { \
            ++neighbors;\
            u32 player = cstate & Cell::PlayerMask; \
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
          if (!(out->state & Cell::AliveMask)) {
            out->setAlive(true);
            if (new_owner != 0) out->setOwner(new_owner);
          }
        } else {
          out->setAlive(false);
        }
    }
    current_ = next;
}

void Logic::place() {
}

bool Logic::test() {
  return true;
}
