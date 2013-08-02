#include <cstdlib>
#include "Game.h"
#include "Viefeld.h"

IViewport *createGame(int argc, const char *argv[]) {
  if (argc < 2) {
    L("usage:\n\t%s <local_port>\nor\n\t%s <remote_host> <remote_port>\n",
      argv[0], argv[0]);
    return nullptr;
  }

  if (argc == 2) {
    KP_LOG_OPEN("proto-server.log");
    int port = atoi(argv[1]);
    L("Listening on port %d", port);
    return new Game(port);
  } else {
    KP_LOG_OPEN("proto-client.log");
    int port = atoi(argv[2]);
    L("Connecting to host %s on port %d", argv[1], port);
    return new Game(argv[1], port);
  }

  return nullptr;
}

Game::Game(int local_port) {
  logic_.create(vec2i(128), local_port);
  pattern_rotation_ = Rotation0;
  viefeld_ = nullptr;
}

Game::Game(const char *remote_host, int remote_port) {
  logic_.connect(remote_host, remote_port);
  pattern_rotation_ = Rotation0;
  viefeld_ = nullptr;
}

void Game::init(IViewportController* controller, Context *context) {
  ctrl_ = controller;
  context_ = context;
  viefeld_ = new Viefeld(context);
}

void Game::resize(vec2i size) {
  glViewport(0, 0, size.x, size.y);
}

void Game::draw(int ms, float dt) {
  if (logic_.update(ms))
    viefeld_->update(context_, logic_.field(), logic_.player());

  GL_ASSERT
  glClear(GL_COLOR_BUFFER_BIT);
  GL_ASSERT
  viefeld_->draw(context_);
  ctrl_->requestRedraw();
}

void Game::close() {
  delete viefeld_;
}

void Game::inputKey(const KeyState &keys) {
  if (keys.isKeyPressed(KeyState::KeyEsc)) ctrl_->quit(0);
  if (!keys.isLastKeyPressed()) return;
  switch (keys.getLastKey()) {
  case KeyState::KeyQ:
    pattern_rotation_ = (pattern_rotation_ + 1) & 3;
    break;
  case KeyState::KeyE:
    pattern_rotation_ = (pattern_rotation_ + 1) & 3;
    break;
  }
}

void Game::inputPointer(const PointerState& pointers) {
  if (pointers.wasPressed()) {
    logic_.place(screenToWorld(pointers.main().getPosition()), static_cast<Rotation>(pattern_rotation_), 2);
  }
}

vec2i Game::screenToWorld(vec2f screen) {
  return vec2i(vec2f(logic_.field().getSize()) * (screen * .5 + vec2f(.5)));
}
