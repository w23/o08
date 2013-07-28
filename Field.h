#pragma once
#include "common.h"

using namespace kapusha;

enum Rotation {
  Rotation0, Rotation90, Rotation180, Rotation270
};

class Field {
public:
  struct Cell {
    enum {
      AliveMask = 0x80,
      OwnerMask = MAX_PLAYERS - 1
      /// \todo TTNMask = 0xff00
      /// \todo EnergyMask = 0x00ff0000
      /// \todo FogMask = 0xff000000
    };
    u32 state;

    inline void setAlive(bool alive) {
      state = (state & ~AliveMask) | (alive ? AliveMask:0);
    }
    inline void setOwner(u32 owner) {
      state = (state & ~OwnerMask) | (owner & OwnerMask) | (0x01000000 << owner);
    }
    inline bool isAlive() const { return (state & 0x80) != 0; }
    inline bool isVisibleTo(u32 player) const {
      return 0 != (state & (0x01000000 << player));
    }
    inline u32 getOwner() const { return state & OwnerMask; }
  };

  struct Player {
    u32 area;
    u32 cells;
    u32 births, deaths;
    /// \todo int d_energy
  };

public:
  Field();
  ~Field();

  void reset(vec2i size);
  void mark(vec2i pos, u32 radius, u32 player);
  bool place(vec2i pos, Rotation rotation, u32 player, vec2i size, const u8 *map);
  void calcNextGeneration();

  inline const vec2i getSize() const { return size_; }
  inline const Cell* getCells() const { return cells_ + current_ * frame_; }
  inline const Cell& getCell(vec2i p) const { return getCells()[size_.x * p.y + p.x]; }
  inline const Player &getPlayer(u32 player) const { return players_[player]; }

  inline Cell* getCells() { return cells_ + current_ * frame_; }
  inline Cell& getCell(vec2i p) { return getCells()[size_.x * p.y + p.x]; }
  inline Player &getPlayer(u32 player) { return players_[player]; }

private:
  vec2i size_;
  u32 frame_;
  u32 current_;
  Cell *cells_;

  Player players_[MAX_PLAYERS];
}; // class Field
