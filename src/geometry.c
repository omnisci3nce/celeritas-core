#include <celeritas.h>

typedef struct static_3d_vert {
  vec4 pos;
  vec4 norm;
  vec2 uv;
  vec2 pad;
} static_3d_vert;

vertex_desc static_3d_vertex_format() {
  vertex_desc desc;
  desc.label = "Static 3D Vertex";
  desc.attributes[0] = ATTR_F32x4; // position
  desc.attributes[1] = ATTR_F32x4; // normal
  desc.attributes[2] = ATTR_F32x2; // tex coord
  desc.attribute_count = 3;
  desc.padding = 16; // 16 bytes padding

  return desc;
}

geometry geo_cuboid(f32 x_scale, f32 y_scale, f32 z_scale) {
  vec4 BACK_BOT_LEFT = (vec4){ 0, 0, 0, 0 };
  vec4 BACK_BOT_RIGHT = (vec4){ 1, 0, 0, 0 };
  vec4 BACK_TOP_LEFT = (vec4){ 0, 1, 0, 0 };
  vec4 BACK_TOP_RIGHT = (vec4){ 1, 1, 0, 0 };
  vec4 FRONT_BOT_LEFT = (vec4){ 0, 0, 1, 0 };
  vec4 FRONT_BOT_RIGHT = (vec4){ 1, 0, 1, 0 };
  vec4 FRONT_TOP_LEFT = (vec4){ 0, 1, 1, 0 };
  vec4 FRONT_TOP_RIGHT = (vec4){ 1, 1, 1, 0 };

  // allocate the data
  static_3d_vert* vertices = malloc(36 * 64);

  vertices[0] = (static_3d_vert){ .pos = BACK_TOP_RIGHT, .norm = (v3tov4(VEC3_NEG_Z)), .uv = {0, 0}};
  vertices[1] = (static_3d_vert){ .pos = BACK_BOT_LEFT, .norm = v3tov4(VEC3_NEG_Z), .uv = {0, 1} };
  vertices[2] = (static_3d_vert){ .pos = BACK_TOP_LEFT, .norm = v3tov4(VEC3_NEG_Z), .uv = {0, 0} };
  vertices[3] = (static_3d_vert){ .pos = BACK_TOP_RIGHT, .norm = v3tov4(VEC3_NEG_Z), .uv = {1, 0} };
  vertices[4] = (static_3d_vert){ .pos = BACK_BOT_RIGHT, .norm = v3tov4(VEC3_NEG_Z), .uv = {1, 1} };
  vertices[5] = (static_3d_vert){ .pos = BACK_BOT_LEFT, .norm = v3tov4(VEC3_NEG_Z), .uv = {0, 1} };

  // front faces
  vertices[6] = (static_3d_vert){ .pos = FRONT_BOT_LEFT, .norm = v3tov4(VEC3_Z), .uv = {0, 1} };
  vertices[7] = (static_3d_vert){ .pos = FRONT_TOP_RIGHT, .norm = v3tov4(VEC3_Z), .uv = {1, 0} };
  vertices[8] = (static_3d_vert){ .pos = FRONT_TOP_LEFT, .norm = v3tov4(VEC3_Z), .uv = {0, 0} };
  vertices[9] = (static_3d_vert){ .pos = FRONT_BOT_LEFT, .norm = v3tov4(VEC3_Z), .uv = {0, 1} };
  vertices[10] = (static_3d_vert){ .pos = FRONT_BOT_RIGHT, .norm = v3tov4(VEC3_Z), .uv = {1, 1} };
  vertices[11] = (static_3d_vert){ .pos = FRONT_TOP_RIGHT, .norm = v3tov4(VEC3_Z), .uv = {1, 0} };

  // top faces
  vertices[12] = (static_3d_vert){ .pos = BACK_TOP_LEFT, .norm = v3tov4(VEC3_Y), .uv = {0, 0} };
  vertices[13] = (static_3d_vert){ .pos = FRONT_TOP_LEFT, .norm = v3tov4(VEC3_Y), .uv = {0, 1} };
  vertices[14] = (static_3d_vert){ .pos = FRONT_TOP_RIGHT, .norm = v3tov4(VEC3_Y), .uv = {1, 1} };
  vertices[15] = (static_3d_vert){ .pos = BACK_TOP_LEFT, .norm = v3tov4(VEC3_Y), .uv = {0, 0} };
  vertices[16] = (static_3d_vert){ .pos = FRONT_TOP_RIGHT, .norm = v3tov4(VEC3_Y), .uv = {1, 1} };
  vertices[17] = (static_3d_vert){ .pos = BACK_TOP_RIGHT, .norm = v3tov4(VEC3_Y), .uv = {1, 0} };

  // bottom faces
  vertices[18] = (static_3d_vert){ .pos = BACK_BOT_LEFT, .norm = v3tov4(VEC3_NEG_Y), .uv = {0, 1} };
  vertices[19] = (static_3d_vert){ .pos = FRONT_BOT_RIGHT, .norm = v3tov4(VEC3_NEG_Y), .uv = {1, 1} };
  vertices[20] = (static_3d_vert){ .pos = FRONT_BOT_LEFT, .norm = v3tov4(VEC3_NEG_Y), .uv = {0, 1} };
  vertices[21] = (static_3d_vert){ .pos = BACK_BOT_LEFT, .norm = v3tov4(VEC3_NEG_Y), .uv = {0, 1} };
  vertices[22] = (static_3d_vert){ .pos = BACK_BOT_RIGHT, .norm = v3tov4(VEC3_NEG_Y), .uv = {1, 1} };
  vertices[23] = (static_3d_vert){ .pos = FRONT_BOT_RIGHT, .norm = v3tov4(VEC3_NEG_Y), .uv = {0, 1} };

  // right faces
  vertices[24] = (static_3d_vert){ .pos = FRONT_TOP_RIGHT, .norm = v3tov4(VEC3_X), .uv = {0, 0} };
  vertices[25] = (static_3d_vert){ .pos = BACK_BOT_RIGHT, .norm = v3tov4(VEC3_X), .uv = {1, 1} };
  vertices[26] = (static_3d_vert){ .pos = BACK_TOP_RIGHT, .norm = v3tov4(VEC3_X), .uv = {1, 0} };
  vertices[27] = (static_3d_vert){ .pos = BACK_BOT_RIGHT, .norm = v3tov4(VEC3_X), .uv = {1, 1} };
  vertices[28] = (static_3d_vert){ .pos = FRONT_TOP_RIGHT, .norm = v3tov4(VEC3_X), .uv = {0, 0} };
  vertices[29] = (static_3d_vert){ .pos = FRONT_BOT_RIGHT, .norm = v3tov4(VEC3_X), .uv = {0, 1} };

  // left faces
  vertices[30] = (static_3d_vert){ .pos = FRONT_TOP_LEFT, .norm = v3tov4(VEC3_NEG_X), .uv = {0, 0} };
  vertices[31] = (static_3d_vert){ .pos = BACK_TOP_LEFT, .norm = v3tov4(VEC3_NEG_X), .uv = {0, 0} };
  vertices[32] = (static_3d_vert){ .pos = BACK_BOT_LEFT, .norm = v3tov4(VEC3_NEG_X), .uv = {0, 0} };
  vertices[33] = (static_3d_vert){ .pos = BACK_BOT_LEFT, .norm = v3tov4(VEC3_NEG_X), .uv = {0, 0} };
  vertices[34] = (static_3d_vert){ .pos = FRONT_BOT_LEFT, .norm = v3tov4(VEC3_NEG_X), .uv = {0, 0} };
  vertices[35] = (static_3d_vert){ .pos = FRONT_TOP_LEFT, .norm = v3tov4(VEC3_NEG_X), .uv = {0, 0} };

  return (geometry) {
    .vertex_format = static_3d_vertex_format(),
    .vertex_data = vertices,
    .has_indices = false,
    .indices = NULL
  };
}