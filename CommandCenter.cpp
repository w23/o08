#include <cstdlib>
#include <cstring>
#include <kapusha/core.h>
#include "CommandCenter.h"

CommandCenter::CommandCenter() { reset(); }
CommandCenter::~CommandCenter() {}

void CommandCenter::set_active_players(u32 players, u32 local) {
  active_players_ = players;
  inactive_players_mask_ = 0xffffffffUL;
  for (u32 i = 1; i <= players; ++i)
    inactive_players_mask_ ^= 1 << i;
  inactive_players_mask_ |= 1 << local;
  L("mask %08x", inactive_players_mask_);
}

void CommandCenter::reset() {
  generation_ = 0;
  memset(generations_, 0, sizeof(generations_));

  // there will be no events for these generations
  for (int i = 0; i < NET_LATENCY_LOCAL; ++i)
    generations_[i].sync_flags = 0xffffffffUL;
  generations_[NET_LATENCY_LOCAL].sync_flags = inactive_players_mask_;
}


Command *CommandCenter::get_new_command_slot(u32 player) {
  Generation *g = get_generation(generation_ + NET_LATENCY_LOCAL);
  if (!g) return nullptr;
  if (g->player[player].n_commands == MAX_PLAYER_COMMANDS) return nullptr;
  return g->player[player].commands + (g->player[player].n_commands++);
}

void CommandCenter::write_remote_commands(
  u32 player, u32 generation,
  u32 n_commands, const Command *commands) {
  Generation *g = get_generation(generation);
  if (!g) {
    L("COMCENTER FATAL: borken remote generation");
    abort();
  }
  if (g->player[player].n_commands != 0) {
    L("COMCENTER FATAL: Conflicting remote command");
    abort();
  }
  g->player[player].n_commands = n_commands;
  memcpy(g->player[player].commands, commands, n_commands * sizeof(Command));
  g->sync_flags |= 1 << player;
}

const Command *CommandCenter::get_new_commands(u32 player, u32 &count) {
  Generation *g = get_generation(generation_ + NET_LATENCY_LOCAL);
  if (!g) return nullptr;
  count = g->player[player].n_commands;
  return g->player[player].commands;
}
 
bool CommandCenter::can_advance() const {
  //L("sync flags: %08x", generations_[(generation_ + 1) % NET_LATENCY].sync_flags);
  //for (int i = 0; i < 8; ++i)
  //  L("%d: sync flags: %08x", i+generation_, generations_[(generation_ + i) % NET_LATENCY].sync_flags); 
  return generations_[(generation_ + 1) % NET_LATENCY].sync_flags == 0xffffffffUL;
}

void CommandCenter::advance() {
  KP_ASSERT(can_advance());
  memset(generations_ + generation_ % NET_LATENCY, 0, sizeof(Generation));
  ++generation_;

  //L("advance %d", generation_);
  
  Generation *newg = generations_ + (generation_ + NET_LATENCY_LOCAL) % NET_LATENCY;
  newg->sync_flags |= inactive_players_mask_;

  Generation *oldg = generations_ + generation_ % NET_LATENCY;
  CommandEx *pcmd = oldg->commands_combined;
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    u32 npcmd = oldg->player[i].n_commands;
    //L("player %d commands %d", i, npcmd);
    for (u32 j = 0; j < npcmd; ++j) {
      pcmd[j].player = i;
      pcmd[j].cmd = oldg->player[i].commands[j];
    }
    pcmd += npcmd;
    oldg->n_commands_combined += npcmd;
  }
}

const CommandEx *CommandCenter::get_current_generation_command(u32 index) const {
  const Generation *g = generations_ + generation_ % NET_LATENCY;
  if (index > g->n_commands_combined) return nullptr;
  return g->commands_combined + index;
}

CommandCenter::Generation *CommandCenter::get_generation(u32 generation) {
  if (generation <= generation_) {
    L("ERROR: Invalid remote generation %d while local = %d", generation, generation_);
    return nullptr;
  }

  /// \todo overfloating generations
  if ((generation - generation_) >= NET_LATENCY) {
    L("FATAL: Remote generation %d is too far into the future past local %d",
      generation, generation_);
    return nullptr;
  }

  return generations_ + generation % NET_LATENCY;
}

