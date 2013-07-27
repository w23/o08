#include <cstring>
#include <kapusha/core.h>
#include "CommandCenter.h"

enum NetworkCode {
  /// \todo GameGenerationReady
  GameCommand = 3
};

struct NetworkCommand {
  u32 generation;
  Command cmd;
};

CommandCenter::CommandCenter() : generation_(0), net_(31337) {
  memset(generations_, 0, sizeof(generations_));
}

CommandCenter::~CommandCenter() {
}

void CommandCenter::sendCommand(const Command &command, u32 generation) {
  Generation &g = generations_[generation % NET_LATENCY];
  KP_ASSERT(g.n_commands < MAX_COMMANDS);
  memcpy(g.commands + g.n_commands, &command, sizeof(Command));
  ++g.n_commands;
}

void CommandCenter::update() {
  u8 packet[MAX_NET_PAYLOAD];
  for(;;) {
    u32 type = net_.receive(packet);
    if (type == 0) break;
    switch (type) {
    case GameCommand:
      {
        const NetworkCommand *netcmd = reinterpret_cast<const NetworkCommand*>(packet);
        KP_ASSERT(generation_ < netcmd->generation);
        KP_ASSERT((netcmd->generation - generation_) < NET_LATENCY);
        sendCommand(netcmd->cmd, netcmd->generation);
        break;
      }
    }
  }
}

void CommandCenter::nextGeneration() {
  memset(generations_ + generation_ % NET_LATENCY, 0, sizeof(Generation));
  ++generation_;
}

u32 CommandCenter::getCommandCount() const {
  return generations_[generation_ % NET_LATENCY].n_commands;
}

const Command *CommandCenter::getCommands() {
  return generations_[generation_ % NET_LATENCY].commands;
}
