#include <cstring>
#include <kapusha/core.h>
#include "CommandCenter.h"

CommandCenter::CommandCenter() { reset(); }
CommandCenter::~CommandCenter() {}

CommandCenter::Generation *CommandCenter::get_generation(u32 generation) {
  if (generation <= generation_) {
    L("ERROR: Invalid remote generation %d while local = %d", generation, generation_);
    return nullptr;
  }
  if ((generation - generation_) >= NET_LATENCY) {
    L("FATAL: Remote generation %d is too far into the future past local %d",
      generation, generation_);
    return nullptr;
  }

  return generations_ + generation % NET_LATENCY;
}

void CommandCenter::reset() {
  generation_ = 0;
  memset(generations_, 0, sizeof(generations_));
}

Command *CommandCenter::get_local_command_slot() {
  Generation &g = generations_[(generation_ + NET_LATENCY_LOCAL) % NET_LATENCY];
  if (g.n_local_commands == MAX_PLAYER_COMMANDS) return nullptr;
  return g.local_commands + g.n_local_commands++;
}

Command *CommandCenter::get_remote_command_slot(u32 generation) {
  Generation *g = get_generation(generation);
  if (!g) return nullptr;

  KP_ASSERT(g->n_commands < MAX_COMMANDS);
  return g->commands + g->n_commands++;
}

void CommandCenter::sync_generation(u32 generation, u32 player) {
  Generation *g = get_generation(generation);
  if (!g) return;
  g->sync_flags |= 1 << (player - 1);
}

bool CommandCenter::next_generation() {
  Generation &g = generations_[generation_ % NET_LATENCY];
  if (g.sync_flags != 0x03) return false; /// \todo generalize for arb. num of players

  Generation &dg = generations_[due_generation() % NET_LATENCY];
  if (dg.n_commands + dg.n_local_commands > MAX_COMMANDS) {
    L("FATAL: Too many commands collected!");
    return false;
  }
  memcpy(dg.commands + dg.n_commands, dg.local_commands,
    dg.n_local_commands * sizeof(Command));
  dg.n_commands += dg.n_local_commands;

  memset(&g, 0, sizeof(Generation));
  ++generation_;
  return true;
}

const Command *CommandCenter::get_current_generation_command(u32 index) const {
  const Generation &g = generations_[generation_ % NET_LATENCY];
  if (index >= g.n_commands) return nullptr;
  return g.commands + index;
}

const Command *CommandCenter::get_due_generation_command(u32 index) const {
  const Generation &g = generations_[due_generation() % NET_LATENCY];
  if (index >= g.n_local_commands) return nullptr;
  return g.local_commands + index;
}
