/**
 * @file maths.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-24
 * @copyright Copyright (c) 2024
 */
#pragma once

#include <math.h>
#include "maths_types.h"

// --- Helpers
#define deg_to_rad(x) (x * 3.14 / 180.0)
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

// --- Vector Implementations

// Dimension 3
static inline vec3 vec3_create(f32 x, f32 y, f32 z) { return (vec3){ x, y, z }; }
#define vec3(x, y, z) (vec3_create(x, y, z))
static inline vec3 vec3_add(vec3 a, vec3 b) { return (vec3){ a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline vec3 vec3_sub(vec3 a, vec3 b) { return (vec3){ a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline vec3 vec3_mult(vec3 a, f32 s) { return (vec3){ a.x * s, a.y * s, a.z * s }; }
static inline vec3 vec3_div(vec3 a, f32 s) { return (vec3){ a.x / s, a.y / s, a.z / s }; }

static inline f32 vec3_len_squared(vec3 a) { return (a.x * a.x) + (a.y * a.y) + (a.z * a.z); }
static inline f32 vec3_len(vec3 a) { return sqrtf(vec3_len_squared(a)); }
static inline vec3 vec3_negate(vec3 a) { return (vec3){ -a.x, -a.y, -a.z }; }
static inline vec3 vec3_normalise(vec3 a) {
  f32 length = vec3_len(a);
  return vec3_div(a, length);
}

static inline f32 vec3_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline vec3 vec3_cross(vec3 a, vec3 b) {
  return (
      vec3){ .x = a.y * b.z - a.z * b.y, .y = a.z * b.x - a.x * b.z, .z = a.x * b.y - a.y * b.x };
}

#define VEC3_ZERO ((vec3){ .x = 0.0, .y = 0.0, .z = 0.0 })
#define VEC3_X ((vec3){ .x = 1.0, .y = 0.0, .z = 0.0 })
#define VEC3_NEG_X ((vec3){ .x = -1.0, .y = 0.0, .z = 0.0 })
#define VEC3_Y ((vec3){ .x = 0.0, .y = 1.0, .z = 0.0 })
#define VEC3_NEG_Y ((vec3){ .x = 0.0, .y = -1.0, .z = 0.0 })
#define VEC3_Z ((vec3){ .x = 0.0, .y = 0.0, .z = 1.0 })
#define VEC3_NEG_Z ((vec3){ .x = 0.0, .y = 0.0, .z = -1.0 })

// TODO: Dimension 2
static inline vec2 vec2_create(f32 x, f32 y) { return (vec2){ x, y }; }

// TODO: Dimension 4
#define VEC4_ZERO ((vec4){ .x = 0.0, .y = 0.0, .z = 0.0, .w = 0.0 })

// --- Quaternion Implementations

static inline f32 quat_dot(quat a, quat b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

static inline quat quat_normalise(quat a) {
  f32 length = sqrtf(quat_dot(a, a)  // same as len squared
  );
  return (quat){ a.x / length, a.y / length, a.z / length, a.w / length };
}

static inline quat quat_ident() { return (quat){ .x = 0.0, .y = 0.0, .z = 0.0, .w = 1.0 }; }

// --- Matrix Implementations

static inline mat4 mat4_ident() {
  return (mat4){ .data = { 1.0, 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1.0 } };
}

static inline mat4 mat4_translation(vec3 position) {
  mat4 out_matrix = mat4_ident();
  out_matrix.data[12] = position.x;
  out_matrix.data[13] = position.y;
  out_matrix.data[14] = position.z;
  return out_matrix;
}

static inline mat4 mat4_scale(f32 scale) {
  mat4 out_matrix = mat4_ident();
  out_matrix.data[0] = scale;
  out_matrix.data[5] = scale;
  out_matrix.data[10] = scale;
  return out_matrix;
}

// TODO: double check this
static inline mat4 mat4_rotation(quat rotation) {
  mat4 out_matrix = mat4_ident();
  quat n = quat_normalise(rotation);

  out_matrix.data[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
  out_matrix.data[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
  out_matrix.data[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

  out_matrix.data[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
  out_matrix.data[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
  out_matrix.data[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

  out_matrix.data[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
  out_matrix.data[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
  out_matrix.data[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

  return out_matrix;
}

static inline mat4 mat4_mult(mat4 lhs, mat4 rhs) {
  mat4 out_matrix = mat4_ident();

  const f32 *m1_ptr = lhs.data;
  const f32 *m2_ptr = rhs.data;
  f32 *dst_ptr = out_matrix.data;

  for (i32 i = 0; i < 4; ++i) {
    for (i32 j = 0; j < 4; ++j) {
      *dst_ptr = m1_ptr[0] * m2_ptr[0 + j] + m1_ptr[1] * m2_ptr[4 + j] + m1_ptr[2] * m2_ptr[8 + j] +
                 m1_ptr[3] * m2_ptr[12 + j];
      dst_ptr++;
    }
    m1_ptr += 4;
  }

  return out_matrix;
}

/** @brief Creates a perspective projection matrix */
static inline mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near_clip,
                                    f32 far_clip) {
  f32 half_tan_fov = tanf(fov_radians * 0.5f);
  mat4 out_matrix = { .data = { 0 } };
  out_matrix.data[0] = 1.0f / (aspect_ratio * half_tan_fov);
  out_matrix.data[5] = 1.0f / half_tan_fov;
  out_matrix.data[10] = -((far_clip + near_clip) / (far_clip - near_clip));
  out_matrix.data[11] = -1.0f;
  out_matrix.data[14] = -((2.0f * far_clip * near_clip) / (far_clip - near_clip));
  return out_matrix;
}

/** @brief Creates an orthographic projection matrix */
static inline mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near_clip,
                                     f32 far_clip) {
  // source: kohi game engine.
  mat4 out_matrix = mat4_ident();

  f32 lr = 1.0f / (left - right);
  f32 bt = 1.0f / (bottom - top);
  f32 nf = 1.0f / (near_clip - far_clip);

  out_matrix.data[0] = -2.0f * lr;
  out_matrix.data[5] = -2.0f * bt;
  out_matrix.data[10] = 2.0f * nf;

  out_matrix.data[12] = (left + right) * lr;
  out_matrix.data[13] = (top + bottom) * bt;
  out_matrix.data[14] = (far_clip + near_clip) * nf;

  return out_matrix;
}

static inline mat4 mat4_look_at(vec3 position, vec3 target, vec3 up) {
  mat4 out_matrix;
  vec3 z_axis;
  z_axis.x = target.x - position.x;
  z_axis.y = target.y - position.y;
  z_axis.z = target.z - position.z;

  z_axis = vec3_normalise(z_axis);
  vec3 x_axis = vec3_normalise(vec3_cross(z_axis, up));
  vec3 y_axis = vec3_cross(x_axis, z_axis);

  out_matrix.data[0] = x_axis.x;
  out_matrix.data[1] = y_axis.x;
  out_matrix.data[2] = -z_axis.x;
  out_matrix.data[3] = 0;
  out_matrix.data[4] = x_axis.y;
  out_matrix.data[5] = y_axis.y;
  out_matrix.data[6] = -z_axis.y;
  out_matrix.data[7] = 0;
  out_matrix.data[8] = x_axis.z;
  out_matrix.data[9] = y_axis.z;
  out_matrix.data[10] = -z_axis.z;
  out_matrix.data[11] = 0;
  out_matrix.data[12] = -vec3_dot(x_axis, position);
  out_matrix.data[13] = -vec3_dot(y_axis, position);
  out_matrix.data[14] = vec3_dot(z_axis, position);
  out_matrix.data[15] = 1.0f;

  return out_matrix;
}

// ...

// --- Transform Implementations

#define TRANSFORM_DEFAULT                                                 \
  ((transform){ .position = VEC3_ZERO,                                    \
                .rotation = (quat){ .x = 0., .y = 0., .z = 0., .w = 0. }, \
                .scale = 1.0,                                             \
                .is_dirty = false })

static transform transform_create(vec3 pos, quat rot, f32 scale) {
  return (transform){ .position = pos, .rotation = rot, .scale = scale, .is_dirty = false };
}

static inline mat4 transform_to_mat(transform *tf) {
  mat4 trans = mat4_translation(tf->position);
  mat4 rot = mat4_rotation(tf->rotation);
  mat4 scale = mat4_scale(tf->scale);
  return mat4_mult(trans, mat4_mult(rot, scale));
}

// --- Sizing asserts

_Static_assert(alignof(vec3) == 4, "vec3 is 4 byte aligned");
_Static_assert(sizeof(vec3) == 12, "vec3 is 12 bytes so has no padding");

_Static_assert(alignof(vec4) == 4, "vec4 is 4 byte aligned");

// --- Some other types
typedef struct u32x3 {
  union {
    struct {
      u32 x;
      u32 y;
      u32 z;
    };
    struct {
      u32 r;
      u32 g;
      u32 b;
    };
  };
} u32x3;
#define u32x3(x, y, z) ((u32x3){ x, y, z })
