#pragma once
#include <kapusha/core.h>

using namespace kapusha;

struct Pattern {
  u8 width, height;
  u16 cost;
  const u8 *map;
};

extern const Pattern g_patterns[4];
