#include "Viefeld.h"
#include "Field.h"

static u32 g_colormap[] = {
  0xff7f7f7f,
  0xff00007f,
  0xff007f00
};

Viefeld::Viefeld(Context* context) {
  colormap_ = new Sampler(Sampler::Nearest, Sampler::Nearest);
  metadata_ = new Sampler(Sampler::Nearest, Sampler::Nearest);
  batch_ = new Batch();
  
  static const char *field_vtx =
#if KAPUSHA_GLES
  "precision lowp float;"
#endif
  "attribute vec4 av4_vertex;\n"
  "varying vec2 vv2_field;\n"
  "void main() {\n"
  "gl_Position = av4_vertex;\n"
  "vv2_field = av4_vertex.xy * .5 + vec2(.5);\n"
  "}";
  static const char *field_frg =
#if KAPUSHA_GLES
  "precision lowp float;"
#endif
  "uniform sampler2D us2_colormap, us2_metadata;\n"
  "varying vec2 vv2_field;\n"
  "void main() {\n"
  "vec4 basecolor = texture2D(us2_colormap, vv2_field);\n"
#if KAPUSHA_GLES
  "vec2 metadata = texture2D(us2_metadata, vv2_field).ra;\n"
#else
  "vec2 metadata = texture2D(us2_metadata, vv2_field).rg;\n"
#endif
  "float visibility = metadata.g;\n"
  "float aliveness = metadata.r;\n"
  "gl_FragColor = (basecolor + vec4(aliveness*.5)) * visibility\n;"
  "}";
  static vec2f fsquad[4] = {
    vec2f(-1.,  1),
    vec2f(-1., -1),
    vec2f( 1.,  1),
    vec2f( 1., -1)
  };
  
  Program *program = new Program(field_vtx, field_frg);
  Material *material = new Material(program);
  material->setUniform("us2_colormap", colormap_.get());
  material->setUniform("us2_metadata", metadata_.get());
  Buffer *buffer = new Buffer(Buffer::BindingArray);
  buffer->load(context, fsquad, sizeof(fsquad));
  batch_->setMaterial(material);
  batch_->setAttribSource("av4_vertex", buffer, 2, 0, sizeof(vec2f));
  batch_->setGeometry(Batch::GeometryTriangleStrip, 0, 4);
}

Viefeld::~Viefeld() {
}

void Viefeld::update(Context* context, const Field& field, u32 player) {
  if (!scolormap_ || scolormap_->meta().size != field.getSize()) {
    scolormap_ = new Surface(Surface::Meta(field.getSize(),
                                           Surface::Meta::RGBA8888));
    smetanadata_ = new Surface(Surface::Meta(field.getSize(),
                                             Surface::Meta::RG88));
  }
  u32 *pcolor = scolormap_->pixels<u32>();
  u16 *pmeta = smetanadata_->pixels<u16>(); /// \todo fix non power of 2 stride
  const Field::Cell *pcell = field.getCells();
  for (int y = 0; y < field.getSize().y; ++y)
    for (int x = 0; x < field.getSize().x; ++x, ++pmeta, ++pcolor, ++pcell) {
      *pcolor = g_colormap[pcell->getOwner()];
      *pmeta = (pcell->isVisibleTo(player)?0xff00:0)
        | (pcell->isAlive()?0x00ff:0);
    }
  colormap_->upload(context, scolormap_.get());
  metadata_->upload(context, smetanadata_.get());
}

void Viefeld::draw(Context* context) const {
  if (scolormap_)
    batch_->draw(context);
}
