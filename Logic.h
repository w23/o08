#pragma once
#include "common.h"
#include "CommandCenter.h"
#include "Field.h"
#include "Network.h"

class Logic {
public:
  enum State {
    Idle = 0,
    Listening,
    Connecting,
    Paused,
    Playing,
    Ended
  };

  Logic();
  ~Logic();

  // Bootstrap
  void create(vec2i size, int listen_port);
  void connect(const char *remote_host, int remote_port);
  State state() const { return state_; }

  // Gameplay
  void place(vec2i pos, Rotation rotation, u32 pattern_index);
  void update(u32 now_ms);

  inline const Field& field() const { return field_; }

private:
  void reset(vec2i size, u32 player);
  void update_network();
  void processCommand(const CommandEx &command);
  void cmdPlace(u32 player, vec2i pos, u32 rot, u32 ipat);

  State state_;
  u32 player_;
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
