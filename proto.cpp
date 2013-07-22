#include "Logic.h"
#include <kapusha/kapusha.h>
#include <kapusha/sys/x11/x11.h>

using namespace kapusha;

class Game : public IViewport {
public:
  Game();
  virtual ~Game() {}
  virtual void init(IViewportController* controller, Context *context);
  virtual void resize(vec2i size);
  virtual void draw(int ms, float dt);
  virtual void close();

private:
  IViewportController *ctrl_;
  Context *context_;
  Logic logic_;

  SSampler fieldsampler_;
  SBatch fieldbatch_;
};

Game::Game() : logic_(vec2i(256)) {
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
      "int state = int(texture2D(us2_field, vv2_field) * 255.);\n"
      "float k = (state > 127) ? 1. : .4;\n"
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
  logic_.step();
  fieldsampler_->upload(context_, Surface::Meta(logic_.getSize(), Surface::Meta::RGBA8888), logic_.getCells());
  
  glClear(GL_COLOR_BUFFER_BIT);
  fieldbatch_->draw(context_);
  ctrl_->requestRedraw();
}

void Game::close() {
}

int main(int argc, char *argv[]) {
  return X11Run(new Game, vec2i(1280, 720), false);
}
