#pragma once
#include <kapusha/math.h>
#include "config.h"

using namespace kapusha;

struct Command {
  u32 code;
  struct {
    u32 generation;
    u32 player;
    union {
      struct {
        vec2i position;
        u32 rotation;
        u32 pattern;
      } place;
    };
  } payload;
};

class CommandCenter {
public:
  CommandCenter();
  ~CommandCenter();

  void reset();
  Command *get_local_command_slot();
  Command *get_remote_command_slot(u32 generation);
  void sync_generation(u32 generation, u32 player);
  bool next_generation();
  const Command *get_current_generation_command(u32 index) const;
  const Command *get_due_generation_command(u32 index) const;
  inline u32 generation() const { return generation_; }
  inline u32 due_generation() const { return generation_ + NET_LATENCY_LOCAL - 1; }

private:
  struct Generation {
    u32 sync_flags;
    u32 n_commands, n_local_commands;
    Command local_commands[MAX_PLAYER_COMMANDS];
    Command commands[MAX_COMMANDS];
  } generations_[NET_LATENCY];
  Generation *get_generation(u32 generation);
  u32 generation_;
};
