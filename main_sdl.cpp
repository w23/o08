#include <iostream>
#include <kapusha/core.h>
#include <kapusha/sys/sdl/KPSDL.h>

using namespace kapusha;

extern IViewport *createGame(int argc, const char *argv[]);

int main(int argc, char *argv[]) {
  IViewport *game = createGame(argc, const_cast<const char**>(argv));

  if (game) return KPSDL(game, 800, 600, false);
  return -1;
}
