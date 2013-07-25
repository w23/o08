#include <cstring>
#include <kapusha/core.h>
#include "CommandCenter.h"

CommandCenter::CommandCenter() : pointer_(0) {
  memset(generations_, 0, sizeof(generations_));
}

CommandCenter::~CommandCenter() {
}

void CommandCenter::sendCommand(const Command &command) {
  Generation &g = generations_[(pointer_ + NET_LATENCY - 1) % NET_LATENCY];
  KP_ASSERT(g.n_commands < MAX_COMMANDS);
  memcpy(g.commands + g.n_commands, &command, sizeof(Command));
  ++g.n_commands;
}

void CommandCenter::update() {
  /// \todo net
}

void CommandCenter::nextGeneration() {
  memset(generations_ + pointer_, 0, sizeof(Generation));
  pointer_ = (pointer_ + 1) % NET_LATENCY;
}

u32 CommandCenter::getCommandCount() const {
  return generations_[pointer_].n_commands;
}

const Command *CommandCenter::getCommands() {
  return generations_[pointer_].commands;
}
