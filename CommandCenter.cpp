#include <cstring>
#include "CommandCenter.h"

CommandCenter::CommandCenter() : pointer_(0) {
  memset(generations_, 0, sizeof(generations_));
}

CommandCenter::~CommandCenter() {
}

void CommandCenter::sendCommand(const Command &command) {
}

void CommandCenter::update() {
}

void CommandCenter::nextGeneration() {
}

u32 CommandCenter::getCommandCount() const {
  return pointer_;
}

const Command *CommandCenter::getCommands() {
  return nullptr;
}
