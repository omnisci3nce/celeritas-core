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
    .colour = (rgba){ 0, 0, 0, 1 }
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

vec3 spherical_to_cartesian_coords(f32 rho, f32 theta, f32 phi) {
  f32 x = rho * sin(phi) * cos(theta);
  f32 y = rho * sin(phi) * sin(theta);
  f32 z = rho * cos(phi);
  return vec3(x, y, z);
}

geometry_data geo_create_uvsphere(f32 radius, u32 north_south_lines, u32 east_west_lines) {
  assert(east_west_lines >= 3);  // sphere will be degenerate and look gacked without at least 3
  assert(north_south_lines >= 3);

  vertex_darray* vertices = vertex_darray_new(2 + (east_west_lines - 1) * north_south_lines);

  // Create a UV sphere with spherical coordinates
  // a point P on the unit sphere can be represented P(r, theta, phi)
  // for each vertex we must convert that to a cartesian R3 coordinate

  // Top point
  vertex top = { .static_3d = { .position = vec3(0, 0, radius),
                                .normal = vec3(0, 0, radius),
                                .tex_coords = vec2(0, 0) } };

  // parallels
  for (u32 i = 0; i < (east_west_lines - 1); i++) {
    // phi should range from 0 to pi
    f32 phi = PI * ((i + 1) / east_west_lines);

    // meridians
    for (u32 j = 0; j < east_west_lines; j++) {
      // theta should range from 0 to 2PI
      f32 theta = TAU * (j / north_south_lines);
      vec3 position = spherical_to_cartesian_coords(radius, theta, phi);
      f32 d = vec3_len(position);
      assert(d == radius);  // all points on the sphere should be 'radius' away from the origin
      vertex v = { .static_3d = {
                       .position = position,
                       .normal = position,       // normal vector on sphere is same as position
                       .tex_coords = vec2(0, 0)  // TODO
                   } };
      vertex_darray_push(vertices, v);
    }
  }

  // Bottom point
  vertex bot = { .static_3d = { .position = vec3(0, 0, -radius),
                                .normal = vec3(0, 0, -radius),
                                .tex_coords = vec2(0, 0) } };

  // TODO: generate indices for for each flat quad on the UV sphere and triangles
  //       where they meet the north and south poles
  u32_darray* indices = u32_darray_new(1);

  geometry_data geo = {
    .format = VERTEX_STATIC_3D,
    .vertices = vertices,
    .has_indices = true,
    .indices = indices,
    .colour = RED_800,
  };
  return geo;
}
