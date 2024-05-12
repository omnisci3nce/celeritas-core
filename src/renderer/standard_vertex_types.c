#include "ral.h"
#include "ral_types.h"
#include "render.h"

vertex_description static_3d_vertex_description() {
  vertex_description builder = { .debug_label = "vertex" };
  vertex_desc_add(&builder, "position", ATTR_F32x3);
  vertex_desc_add(&builder, "normal", ATTR_F32x3);
  vertex_desc_add(&builder, "texCoords", ATTR_F32x2);
  return builder;
}