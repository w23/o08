#pragma once
#include "config.h"
#include <kapusha/math.h>
#include "CommandCenter.h"

using namespace kapusha;

class Logic {
public:
  Logic(vec2i size);
  ~Logic();

  void update(u32 now_ms);
  void place(vec2i pos, u32 pattern_index);

  struct Cell {
    enum {
      AliveMask = 0x80,
      OwnerMask = MAX_PLAYERS - 1,
      TTNMask = 0xff00
    };
    u32 state;

    inline void setAlive(bool alive) { state = (state & ~AliveMask) | (alive ? AliveMask:0); }
    inline void setOwner(u32 owner) { state = (state & ~OwnerMask) | (owner & OwnerMask); }
    inline bool isAlive() const { return (state & 0x80) != 0; }
    inline u32 getOwner() const { return state & OwnerMask; }
  };

  inline const vec2i getSize() const { return size_; }
  inline const Cell* getCells() const { return cells_ + current_ * frame_; }

  struct Player {
    u32 area;
    u32 cells;
    u32 resources;

    Player() : area(0), cells(0), resources(0) {}
  };

  const Player &getPlayer(u32 player) const { return players_[player]; }

private:
  void processCommand(const Command &command);
  void step();

  vec2i size_;
  u32 frame_;
  u32 current_;
  Cell *cells_;

  Player players_[MAX_PLAYERS];
  u32 generation_;
  u32 nextGenerationTime_;

  CommandCenter comcenter_;

  u32 player_;
}; // class Logic
