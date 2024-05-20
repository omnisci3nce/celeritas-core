#include "ral.h"

size_t vertex_attrib_size(vertex_attrib_type attr) {
  switch (attr) {
    case ATTR_F32:
    case ATTR_U32:
    case ATTR_I32:
      return 4;
    case ATTR_F32x2:
    case ATTR_U32x2:
    case ATTR_I32x2:
      return 8;
    case ATTR_F32x3:
    case ATTR_U32x3:
    case ATTR_I32x3:
      return 12;
    case ATTR_F32x4:
    case ATTR_U32x4:
    case ATTR_I32x4:
      return 16;
      break;
  }
}

void vertex_desc_add(vertex_description* builder, const char* name, vertex_attrib_type type) {
  u32 i = builder->attributes_count;

  size_t size = vertex_attrib_size(type);
  builder->attributes[i] = type;
  builder->stride += size;
  builder->attr_names[i] = name;

  builder->attributes_count++;
}

vertex_description static_3d_vertex_description() {
  vertex_description builder = { .debug_label = "vertex" };
  vertex_desc_add(&builder, "position", ATTR_F32x3);
  vertex_desc_add(&builder, "normal", ATTR_F32x3);
  vertex_desc_add(&builder, "texCoords", ATTR_F32x2);
  return builder;
}
