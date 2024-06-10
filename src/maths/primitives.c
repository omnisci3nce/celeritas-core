#include "primitives.h"
#include "colours.h"
#include "maths.h"
#include "ral_types.h"
#include "render_types.h"

// TODO: move to another file
void geo_free_data(geometry_data* geo) {
  vertex_darray_free(geo->vertices);
  geo->vertices = NULL;
  // TODO: do indices as well
  /* if (geo->has_indices) { */
  /*   u32_darray_free(&geo->indices); */
  /* } */
}

// vertices
f32 plane_vertex_positions[] = {
  // triangle 1
  -0.5, 0, -0.5, -0.5, 0, 0.5, 0.5, 0, -0.5,
  // triangle 2
  0.5, 0, -0.5, -0.5, 0, 0.5, 0.5, 0, 0.5
};

geometry_data geo_create_plane(f32x2 extents) {
  f32x2 half_extents = vec2_div(extents, 2.0);
  vertex_format format = VERTEX_STATIC_3D;
  vertex_darray* vertices = vertex_darray_new(4);

  //   vertex_darray_push(vertices, (vertex){ .static_3d = { .position = } });

  //   return (geometry_data) { .format = format, .vertices =.has_indices = true, }
}

// OLD

static const vec3 BACK_BOT_LEFT = (vec3){ 0, 0, 0 };
static const vec3 BACK_BOT_RIGHT = (vec3){ 1, 0, 0 };
static const vec3 BACK_TOP_LEFT = (vec3){ 0, 1, 0 };
static const vec3 BACK_TOP_RIGHT = (vec3){ 1, 1, 0 };
static const vec3 FRONT_BOT_LEFT = (vec3){ 0, 0, 1 };
static const vec3 FRONT_BOT_RIGHT = (vec3){ 1, 0, 1 };
static const vec3 FRONT_TOP_LEFT = (vec3){ 0, 1, 1 };
static const vec3 FRONT_TOP_RIGHT = (vec3){ 1, 1, 1 };

#define VERT_3D(arr, pos, norm, uv)                                                    \
  {                                                                                    \
    vertex v = { .static_3d = { .position = pos, .normal = norm, .tex_coords = uv } }; \
    vertex_darray_push(arr, v);                                                        \
  }

geometry_data geo_create_cuboid(f32x3 extents) {
  /* static mesh prim_cube_mesh_create() { */
  vertex_darray* vertices = vertex_darray_new(36);

  // back faces
  VERT_3D(vertices, BACK_TOP_RIGHT, VEC3_NEG_Z, vec2(1, 0));
  VERT_3D(vertices, BACK_BOT_LEFT, VEC3_NEG_Z, vec2(0, 1));
  VERT_3D(vertices, BACK_TOP_LEFT, VEC3_NEG_Z, vec2(0, 0));
  VERT_3D(vertices, BACK_TOP_RIGHT, VEC3_NEG_Z, vec2(1, 0));
  VERT_3D(vertices, BACK_BOT_RIGHT, VEC3_NEG_Z, vec2(1, 1));
  VERT_3D(vertices, BACK_BOT_LEFT, VEC3_NEG_Z, vec2(0, 1));

  // front faces
  VERT_3D(vertices, FRONT_BOT_LEFT, VEC3_Z, vec2(0, 1));
  VERT_3D(vertices, FRONT_TOP_RIGHT, VEC3_Z, vec2(1, 0));
  VERT_3D(vertices, FRONT_TOP_LEFT, VEC3_Z, vec2(0, 0));
  VERT_3D(vertices, FRONT_BOT_LEFT, VEC3_Z, vec2(0, 1));
  VERT_3D(vertices, FRONT_BOT_RIGHT, VEC3_Z, vec2(1, 1));
  VERT_3D(vertices, FRONT_TOP_RIGHT, VEC3_Z, vec2(1, 0));

  // top faces
  VERT_3D(vertices, BACK_TOP_LEFT, VEC3_Y, vec2(0, 0));
  VERT_3D(vertices, FRONT_TOP_LEFT, VEC3_Y, vec2(0, 1));
  VERT_3D(vertices, FRONT_TOP_RIGHT, VEC3_Y, vec2(1, 1));
  VERT_3D(vertices, BACK_TOP_LEFT, VEC3_Y, vec2(0, 0));
  VERT_3D(vertices, FRONT_TOP_RIGHT, VEC3_Y, vec2(1, 1));
  VERT_3D(vertices, BACK_TOP_RIGHT, VEC3_Y, vec2(1, 0));

  // bottom faces
  VERT_3D(vertices, BACK_BOT_LEFT, VEC3_NEG_Y, vec2(0, 1));
  VERT_3D(vertices, FRONT_BOT_RIGHT, VEC3_NEG_Y, vec2(1, 1));
  VERT_3D(vertices, FRONT_BOT_LEFT, VEC3_NEG_Y, vec2(0, 1));
  VERT_3D(vertices, BACK_BOT_LEFT, VEC3_NEG_Y, vec2(0, 1));
  VERT_3D(vertices, BACK_BOT_RIGHT, VEC3_NEG_Y, vec2(1, 1));
  VERT_3D(vertices, FRONT_BOT_RIGHT, VEC3_NEG_Y, vec2(0, 1));

  // right faces
  VERT_3D(vertices, FRONT_TOP_RIGHT, VEC3_X, vec2(0, 0));
  VERT_3D(vertices, BACK_BOT_RIGHT, VEC3_X, vec2(1, 1));
  VERT_3D(vertices, BACK_TOP_RIGHT, VEC3_X, vec2(1, 0));
  VERT_3D(vertices, BACK_BOT_RIGHT, VEC3_X, vec2(1, 1));
  VERT_3D(vertices, FRONT_TOP_RIGHT, VEC3_X, vec2(0, 0));
  VERT_3D(vertices, FRONT_BOT_RIGHT, VEC3_X, vec2(0, 1));

  // left faces
  VERT_3D(vertices, FRONT_TOP_LEFT, VEC3_NEG_X, vec2(0, 0));
  VERT_3D(vertices, BACK_TOP_LEFT, VEC3_NEG_X, vec2(0, 0));
  VERT_3D(vertices, BACK_BOT_LEFT, VEC3_NEG_X, vec2(0, 0));
  VERT_3D(vertices, BACK_BOT_LEFT, VEC3_NEG_X, vec2(0, 0));
  VERT_3D(vertices, FRONT_BOT_LEFT, VEC3_NEG_X, vec2(0, 0));
  VERT_3D(vertices, FRONT_TOP_LEFT, VEC3_NEG_X, vec2(0, 0));

  u32_darray* indices = u32_darray_new(vertices->len);

  for (u32 i = 0; i < vertices->len; i++) {
    u32_darray_push(indices, i);
  }

  geometry_data geo = {
    .format = VERTEX_STATIC_3D,
    .vertices = vertices,
    .has_indices = true,
    .indices = indices,  // FIXME: make darray methods that return stack allocated struct
    .colour = vec3(0, 0, 0),
  };

  return geo;
}

/* /\** @brief create a new model with the shape of a cube *\/ */
/* static model_handle prim_cube_new(core* core) { */
/*   model model = { 0 }; */
/*   mesh cube = prim_cube_mesh_create(); */

/*   mesh_darray_push(model.meshes, cube); */
/*   assert(mesh_darray_len(model.meshes) == 1); */

/*   u32 index = (u32)model_darray_len(core->models); */
/*   model_darray_push_copy(core->models, &model); */
/*   return (model_handle){ .raw = index }; */
/* } */

// --- Spheres

geometry_data geo_create_uvsphere(f32 radius, f32 north_south_lines, f32 east_west_lines) {
  // TODO
}
