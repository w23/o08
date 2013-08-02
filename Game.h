#pragma once
#include "Logic.h"
#include <kapusha/kapusha.h>

using namespace kapusha;

extern IViewport *createGame(int argc, const char *argv[]);

class Viefeld;
class Game : public IViewport {
public:
  Game(int local_port);
  Game(const char *remote_host, int remote_port);
  virtual ~Game() {}
  virtual void init(IViewportController* controller, Context *context);
  virtual void resize(vec2i size);
  virtual void draw(int ms, float dt);
  virtual void close();

  virtual void inputKey(const KeyState& keys);
  virtual void inputPointer(const PointerState& pointers);

private:
  IViewportController *ctrl_;
  Context *context_;
  Logic logic_;

  Viefeld *viefeld_;
  vec2i screenToWorld(vec2f screen);

  int pattern_rotation_;
};