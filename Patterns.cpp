#include "Patterns.h"

static const u8 map_block[16] = {
  0, 0, 0, 0,
  0, 1, 1, 0,
  0, 1, 1, 0,
  0, 0, 0, 0
};

static const u8 map_glider[25] = {
  0, 0, 0, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 0, 1, 0,
  0, 1, 1, 1, 0,
  0, 0, 0, 0, 0
};

static const u8 map_blinker[15] = {
  0, 0, 0, 0, 0,
  0, 1, 1, 1, 0,
  0, 0, 0, 0, 0
};

static const u8 map_cell[1] = { 1 };

const Pattern g_patterns[4] = {
  {1, 1, 1,  map_cell},
  {4, 4, 8,  map_block},
  {5, 5, 32, map_glider},
  {5, 3, 16, map_blinker}
};
