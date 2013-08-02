#pragma once
#include <kapusha/render.h>

using namespace kapusha;

class Field;
class Viefeld {
public:
  Viefeld(Context* context);
  ~Viefeld();
  
  void update(Context* context, const Field& field, u32 player);
  void draw(Context* context) const;
  
private:
  SSurface scolormap_, smetanadata_;
  SSampler colormap_, metadata_;
  SBatch batch_;
};