#pragma once

#include <assert.h>
#include <stdlib.h>
#include "core.h"
#include "maths.h"
#include "render_types.h"

static const vec3 BACK_BOT_LEFT = (vec3){ 0, 0, 0 };
static const vec3 BACK_BOT_RIGHT = (vec3){ 1, 0, 0 };
static const vec3 BACK_TOP_LEFT = (vec3){ 0, 1, 0 };
static const vec3 BACK_TOP_RIGHT = (vec3){ 1, 1, 0 };
static const vec3 FRONT_BOT_LEFT = (vec3){ 0, 0, 1 };
static const vec3 FRONT_BOT_RIGHT = (vec3){ 1, 0, 1 };
static const vec3 FRONT_TOP_LEFT = (vec3){ 0, 1, 1 };
static const vec3 FRONT_TOP_RIGHT = (vec3){ 1, 1, 1 };

static mesh prim_cube_mesh_create() {
  mesh cube = { 0 };
  cube.vertices = vertex_darray_new(36);

  // back faces
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_BOT_LEFT, .normal = VEC3_NEG_Z, .uv = (vec2){ 0, 1 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_TOP_LEFT, .normal = VEC3_NEG_Z, .uv = (vec2){ 0, 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_TOP_RIGHT, .normal = VEC3_NEG_Z, .uv = (vec2){ 1, 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_TOP_RIGHT, .normal = VEC3_NEG_Z, .uv = (vec2){ 1, 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_BOT_RIGHT, .normal = VEC3_NEG_Z, .uv = (vec2){ 1,1 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_BOT_LEFT, .normal = VEC3_NEG_Z, .uv = (vec2){ 0, 1 } });

  // front faces
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_BOT_LEFT, .normal = VEC3_Z, .uv = (vec2){ 0, 1} });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_TOP_RIGHT, .normal = VEC3_Z, .uv = (vec2){ 1, 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_TOP_LEFT, .normal = VEC3_Z, .uv = (vec2){ 0, 0} });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_BOT_LEFT, .normal = VEC3_Z, .uv = (vec2){ 0, 1 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_BOT_RIGHT, .normal = VEC3_Z, .uv = (vec2){ 1, 1 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_TOP_RIGHT, .normal = VEC3_Z, .uv = (vec2){ 1, 0 } });

  // top faces
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = BACK_TOP_LEFT, .normal = VEC3_Y, .uv = (vec2){ 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_TOP_LEFT, .normal = VEC3_Y, .uv = (vec2){ 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_TOP_RIGHT, .normal = VEC3_Y, .uv = (vec2){ 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = BACK_TOP_LEFT, .normal = VEC3_Y, .uv = (vec2){ 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_TOP_RIGHT, .normal = VEC3_Y, .uv = (vec2){ 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = BACK_TOP_RIGHT, .normal = VEC3_Y, .uv = (vec2){ 0 } });

  // bottom faces
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_BOT_LEFT, .normal = VEC3_NEG_Y, .uv = (vec2){ 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = FRONT_BOT_RIGHT, .normal = VEC3_NEG_Y, .uv = (vec2){ 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = FRONT_BOT_LEFT, .normal = VEC3_NEG_Y, .uv = (vec2){ 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_BOT_LEFT, .normal = VEC3_NEG_Y, .uv = (vec2){ 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_BOT_RIGHT, .normal = VEC3_NEG_Y, .uv = (vec2){ 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = FRONT_BOT_RIGHT, .normal = VEC3_NEG_Y, .uv = (vec2){ 0 } });

  // right faces
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_TOP_RIGHT, .normal = VEC3_X, .uv = (vec2){ 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = BACK_BOT_RIGHT, .normal = VEC3_X, .uv = (vec2){ 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = BACK_TOP_RIGHT, .normal = VEC3_X, .uv = (vec2){ 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = BACK_BOT_RIGHT, .normal = VEC3_X, .uv = (vec2){ 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_TOP_RIGHT, .normal = VEC3_X, .uv = (vec2){ 0 } });
  vertex_darray_push(cube.vertices,
                     (vertex){ .position = FRONT_BOT_RIGHT, .normal = VEC3_X, .uv = (vec2){ 0 } });

  // left faces
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = FRONT_TOP_LEFT, .normal = VEC3_NEG_X, .uv = (vec2){ 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_TOP_LEFT, .normal = VEC3_NEG_X, .uv = (vec2){ 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_BOT_LEFT, .normal = VEC3_NEG_X, .uv = (vec2){ 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = BACK_BOT_LEFT, .normal = VEC3_NEG_X, .uv = (vec2){ 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = FRONT_BOT_LEFT, .normal = VEC3_NEG_X, .uv = (vec2){ 0 } });
  vertex_darray_push(
      cube.vertices,
      (vertex){ .position = FRONT_TOP_LEFT, .normal = VEC3_NEG_X, .uv = (vec2){ 0 } });

  cube.indices_len = cube.vertices->len;
  cube.indices = malloc(sizeof(u32) * cube.indices_len);

  for (u32 i = 0; i < cube.indices_len; i++) {
    cube.indices[i] = i;
  }

  cube.has_indices = true;

  return cube;
}

/** @brief create a new model with the shape of a cube */
static model_handle prim_cube_new(core* core) {
  model model = { 0 };
  mesh cube = prim_cube_mesh_create();

  mesh_darray_push(model.meshes, cube);
  assert(mesh_darray_len(model.meshes) == 1);

  u32 index = (u32)model_darray_len(core->models);
  model_darray_push_copy(core->models, &model);
  return (model_handle){ .raw = index };
}