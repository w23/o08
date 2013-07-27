#pragma once
#include "common.h"
#include "CommandCenter.h"
#include "Field.h"

class Logic {
public:
  Logic();
  ~Logic();

  void reset(vec2i size);
  void place(vec2i pos, Rotation rotation, u32 pattern_index);
  void update(u32 now_ms);

  inline const Field& field() const { return field_; }

private:
  void processCommand(const Command &command);
  void cmdPlace(u32 player, vec2i pos, u32 rot, u32 ipat);

  u32 player_;
  u32 generation_;
  u32 nextGenerationTime_;

  Field field_;
  CommandCenter comcenter_;
  Network net_;

  struct Player {
    u32 resources;
    u32 area;
    u32 cells;
  } players_[MAX_PLAYERS];
}; // class Logic
