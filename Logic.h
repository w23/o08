#pragma once
#include <kapusha/math.h>

using namespace kapusha;

class Logic {
public:
  Logic(vec2i size);
  ~Logic();

  void step();
  void place();
  bool test();

  struct Cell {
    enum {
      AliveMask = 0x80,
      PlayerMask = 0x07,
      TTNMask = 0xff00
   };
    u32 state;

    inline void setAlive(bool alive) { state = (state & ~AliveMask) | (alive?AliveMask:0); }
    inline void setOwner(u32 player) { state = (state & ~PlayerMask) | (player & PlayerMask); }

  };

  inline const vec2i getSize() const { return size_; }
  inline const Cell* getCells() const { return cells_ + current_ * frame_; }

private:
  vec2i size_;
  u32 frame_;
  u32 current_;
  Cell *cells_;
}; // class Logic
