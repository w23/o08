#pragma once
#include <kapusha/math.h>
#include "config.h"

using namespace kapusha;

struct Command {
  u32 player;
  enum {
    Ready = 0,
    Place = 1
  } type;
  union {
    struct {
      vec2i position;
      u32 rotation;
      u32 pattern;
    } place;
    u32 params[14];
  };
};

class CommandCenter {
public:
  CommandCenter();
  ~CommandCenter();

  void sendCommand(const Command &command);
  void update();
  void nextGeneration();
  /// \todo ! bool checkSync() const; !
  u32 getCommandCount() const;
  const Command *getCommands();

private:
  struct Generation {
    u32 sync_flags;
    u32 n_commands;
    Command commands[MAX_COMMANDS];
  } generations_[NET_LATENCY];
  u32 pointer_;
};
