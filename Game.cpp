#include <cstdlib>
#include "Game.h"

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
}

Game::Game(const char *remote_host, int remote_port) {
  logic_.connect(remote_host, remote_port);
}

void Game::init(IViewportController* controller, Context *context) {
  ctrl_ = controller;
  context_ = context;
  fieldsampler_ = new Sampler(Sampler::Nearest, Sampler::Nearest);
  //fieldsampler_->allocate(context_, Sufrace::Meta(vec2i(128), Surface::Meta::RGBA8888));
  fieldbatch_ = new Batch();

  static const char *field_vtx =
    "attribute vec4 av4_vertex;\n"
    "varying vec2 vv2_field;\n"
    "void main() {\n"
      "gl_Position = av4_vertex;\n"
      "vv2_field = av4_vertex.xy * .5 + vec2(.5);\n"
    "}";
  static const char *field_frg =
    "uniform sampler2D us2_field;\n"
    "vec4 colors[8];\n"
    "varying vec2 vv2_field;\n"
    "void main() {\n"
      "colors[0] = vec4(.7);\n"
      "colors[1] = vec4(1., 0., 0., 1.);\n"
      "colors[2] = vec4(0., 1., 0., 1.);\n"
      "colors[3] = vec4(0., 0., 1., 1.);\n"
      "colors[4] = vec4(1., 1., 0., 1.);\n"
      "colors[5] = vec4(1., 0., 1., 1.);\n"
      "colors[6] = vec4(0., 1., 1., 1.);\n"
      "colors[7] = vec4(1., .5, .5, 1.);\n"
      "vec4 cell = texture2D(us2_field, vv2_field);\n"
      "int state = int(cell.r * 255.);\n"
      "int fog = int(cell.a * 255.);"
      "float k = (state > 127) ? 1. : .4;\n"
      "k *= ((fog & 2) == 2) ? 1. : .2;\n"
      "int player = mod(state, 8);\n"
      "gl_FragColor = colors[player] * k\n;"
      //"gl_FragColor = texture2D(us2_field, vv2_field) * 100.;\n"
    "}";
  static vec2f fsquad[4] = {
    vec2f(-1.,  1),
    vec2f(-1., -1),
    vec2f( 1.,  1),
    vec2f( 1., -1)
  };

  Program *program = new Program(field_vtx, field_frg);
  Material *material = new Material(program);
  material->setUniform("us2_field", fieldsampler_.get());
  Buffer *buffer = new Buffer(Buffer::BindingArray);
  buffer->load(context_, fsquad, sizeof(fsquad));
  fieldbatch_->setMaterial(material);
  fieldbatch_->setAttribSource("av4_vertex", buffer, 2, 0, sizeof(vec2f));
  fieldbatch_->setGeometry(Batch::GeometryTriangleStrip, 0, 4);
}

void Game::resize(vec2i size) {
  glViewport(0, 0, size.x, size.y);
}

void Game::draw(int ms, float dt) {
  logic_.update(ms);
  fieldsampler_->upload(context_, Surface::Meta(logic_.field().getSize(), Surface::Meta::RGBA8888), logic_.field().getCells());

  glClear(GL_COLOR_BUFFER_BIT);
  fieldbatch_->draw(context_);
  ctrl_->requestRedraw();
}

void Game::close() {
  fieldsampler_.reset();
  fieldbatch_.reset();
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
