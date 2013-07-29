#pragma once
#include <kapusha/math.h>
#include "config.h"

using namespace kapusha;

struct Command {
  enum Code : u32 {
    Place
  } code;
  union {
    struct {
        vec2i position;
        u32 rotation;
        u32 pattern;
    } place;
  };
};

struct CommandEx {
  u32 player;
  Command cmd;
};

class CommandCenter {
public:
  CommandCenter();
  ~CommandCenter();

  void set_active_players(u32 players, u32 local);

  void reset();
  Command *get_new_command_slot(u32 player);
  void write_remote_commands(
    u32 player, u32 generation,
    u32 n_commands, const Command *commands);

  const Command *get_new_commands(u32 player, u32 &count);
  
  bool can_advance() const;
  void advance();

  const CommandEx *get_current_generation_command(u32 index) const;
  inline u32 generation() const { return generation_; }
  inline u32 new_generation() const { return generation_ + NET_LATENCY_LOCAL; }

private:
  u32 active_players_;
  u32 inactive_players_mask_;
  struct Generation {
    u32 sync_flags;
    struct Player {
      u32 n_commands;
      Command commands[MAX_PLAYER_COMMANDS];
    } player[MAX_PLAYERS];
    u32 n_commands_combined;
    CommandEx commands_combined[MAX_PLAYER_COMMANDS * MAX_PLAYERS];
  } generations_[NET_LATENCY];
  u32 generation_;

  Generation *get_generation(u32 generation);
};
