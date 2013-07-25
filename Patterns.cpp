#include "Patterns.h"

static const u8 map_block[16] = {
  0, 0, 0, 0,
  0, 1, 1, 0,
  0, 1, 1, 0,
  0, 0, 0, 0
};

const Pattern g_patterns[4] = {
  {4, 4, 16, map_block}
};
