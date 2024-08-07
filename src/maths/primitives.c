#include "primitives.h"
#include "colours.h"
#include "log.h"
#include "maths.h"
#include "ral_types.h"
#include "render_types.h"

// --- Helpers
#define VERT_3D(arr, pos, norm, uv)                                                    \
  {                                                                                    \
    vertex v = { .static_3d = { .position = pos, .normal = norm, .tex_coords = uv } }; \
    vertex_darray_push(arr, v);                                                        \
  }

void push_triangle(u32_darray* arr, u32 i0, u32 i1, u32 i2) {
  u32_darray_push(arr, i0);
  u32_darray_push(arr, i1);
  u32_darray_push(arr, i2);
}

// TODO: move to another file
void geo_free_data(geometry_data* geo) {
  vertex_darray_free(geo->vertices);
  geo->vertices = NULL;
}

vec3 plane_vertex_positions[] = {
  (vec3){ -0.5, 0, -0.5 },
  (vec3){ 0.5, 0, -0.5 },
  (vec3){ -0.5, 0, 0.5 },
  (vec3){ 0.5, 0, 0.5 },
};

geometry_data geo_create_plane(f32x2 extents) {
  vertex_darray* vertices = vertex_darray_new(4);
  u32_darray* indices = u32_darray_new(vertices->len);

  vec3 vert_pos[4];
  memcpy(&vert_pos, plane_vertex_positions, sizeof(plane_vertex_positions));
  for (int i = 0; i < 4; i++) {
    vert_pos[i].x *= extents.x;
    vert_pos[i].z *= extents.y;
  }
  VERT_3D(vertices, vert_pos[0], VEC3_Y, vec2(0, 0));
  VERT_3D(vertices, vert_pos[1], VEC3_Y, vec2(1, 0));
  VERT_3D(vertices, vert_pos[2], VEC3_Y, vec2(0, 1));
  VERT_3D(vertices, vert_pos[3], VEC3_Y, vec2(1, 1));

  /* push_triangle(indices, 0, 1, 2); */
  /* push_triangle(indices, 2, 1, 3); */
  push_triangle(indices, 2, 1, 0);
  push_triangle(indices, 3, 1, 2);

  geometry_data geo = { .format = VERTEX_STATIC_3D,
                        .vertices = vertices,
                        .has_indices = true,
                        .indices = indices,
                        .colour = (rgba){ 0, 0, 0, 1 } };

  return geo;
}

static const vec3 BACK_BOT_LEFT = (vec3){ 0, 0, 0 };
static const vec3 BACK_BOT_RIGHT = (vec3){ 1, 0, 0 };
static const vec3 BACK_TOP_LEFT = (vec3){ 0, 1, 0 };
static const vec3 BACK_TOP_RIGHT = (vec3){ 1, 1, 0 };
static const vec3 FRONT_BOT_LEFT = (vec3){ 0, 0, 1 };
static const vec3 FRONT_BOT_RIGHT = (vec3){ 1, 0, 1 };
static const vec3 FRONT_TOP_LEFT = (vec3){ 0, 1, 1 };
static const vec3 FRONT_TOP_RIGHT = (vec3){ 1, 1, 1 };

geometry_data geo_create_cuboid(f32x3 extents) {
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

// --- Spheres

vec3 spherical_to_cartesian_coords(f32 rho, f32 theta, f32 phi) {
  f32 x = rho * sin(phi) * cos(theta);
  f32 y = rho * cos(phi);
  f32 z = rho * sin(phi) * sin(theta);
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
  vertex top = { .static_3d = { .position = vec3(0, radius, 0),
                                .normal = vec3_normalise(vec3(0, radius, 0)),
                                .tex_coords = vec2(0, 0) } };
  vertex_darray_push(vertices, top);

  // parallels
  for (u32 i = 0; i < (east_west_lines - 1); i++) {
    // phi should range from 0 to pi
    f32 phi = PI * (((f32)i + 1) / (f32)east_west_lines);

    // meridians
    for (u32 j = 0; j < east_west_lines; j++) {
      // theta should range from 0 to 2PI
      f32 theta = TAU * ((f32)j / (f32)north_south_lines);
      vec3 position = spherical_to_cartesian_coords(radius, theta, phi);
      // f32 d = vec3_len(position);
      // print_vec3(position);
      // printf("Phi %f Theta %f d %d\n", phi, theta, d);
      // assert(d == radius);  // all points on the sphere should be 'radius' away from the origin
      vertex v = { .static_3d = {
                       .position = position,
                       .normal =
                           vec3_normalise(position),  // normal vector on sphere is same as position
                       .tex_coords = vec2(0, 0)       // TODO
                   } };
      vertex_darray_push(vertices, v);
    }
  }

  // Bottom point
  vertex bot = { .static_3d = { .position = vec3(0, -radius, 0),
                                .normal = vec3_normalise(vec3(0, -radius, 0)),
                                .tex_coords = vec2(0, 0) } };
  vertex_darray_push(vertices, bot);

  u32_darray* indices = u32_darray_new(1);

  // top bottom rings
  for (u32 i = 0; i < north_south_lines; i++) {
    u32 i1 = i + 1;
    u32 i2 = (i + 1) % north_south_lines + 1;
    push_triangle(indices, 0, i1, i2);
    /* TRACE("Push triangle (%.2f %.2f %.2f)->(%.2f %.2f %.2f)->(%.2f %.2f %.2f)\n", */
    /*       vertices->data[0].static_3d.position.x, vertices->data[0].static_3d.position.y, */
    /*       vertices->data[0].static_3d.position.z, vertices->data[i1].static_3d.position.x, */
    /*       vertices->data[i1].static_3d.position.y, vertices->data[i1].static_3d.position.z, */
    /*       vertices->data[i2].static_3d.position.x, vertices->data[i2].static_3d.position.y, */
    /*       vertices->data[i2].static_3d.position.z); */
    u32 bot = vertices->len - 1;
    u32 i3 = i + north_south_lines * (east_west_lines - 2) + 1;
    u32 i4 = (i + 1) % north_south_lines + north_south_lines * (east_west_lines - 2) + 1;
    push_triangle(indices, i3, bot, i4);
  }

  // quads
  for (u32 i = 0; i < east_west_lines - 2; i++) {
    u32 ring_start = i * north_south_lines + 1;
    u32 next_ring_start = (i + 1) * north_south_lines + 1;
    /* printf("ring start %d next ring start %d\n", ring_start, next_ring_start); */
    /* print_vec3(vertices->data[ring_start].static_3d.position); */
    /* print_vec3(vertices->data[next_ring_start].static_3d.position); */
    for (u32 j = 0; j < north_south_lines; j++) {
      u32 i0 = ring_start + j;
      u32 i1 = next_ring_start + j;
      u32 i2 = ring_start + (j + 1) % north_south_lines;
      u32 i3 = next_ring_start + (j + 1) % north_south_lines;
      push_triangle(indices, i0, i2, i1);
      /* TRACE("Push triangle (%.2f %.2f %.2f)->(%.2f %.2f %.2f)->(%.2f %.2f %.2f)\n", */
      /*       vertices->data[i0].static_3d.position.x, vertices->data[i0].static_3d.position.y, */
      /*       vertices->data[i0].static_3d.position.z, vertices->data[i1].static_3d.position.x, */
      /*       vertices->data[i1].static_3d.position.y, vertices->data[i1].static_3d.position.z, */
      /*       vertices->data[i2].static_3d.position.x, vertices->data[i2].static_3d.position.y, */
      /*       vertices->data[i2].static_3d.position.z); */
      push_triangle(indices, i1, i2, i3);
    }
  }

  geometry_data geo = {
    .format = VERTEX_STATIC_3D,
    .vertices = vertices,
    .has_indices = true,
    .indices = indices,
    .colour = RED_800,
  };
  return geo;
}
