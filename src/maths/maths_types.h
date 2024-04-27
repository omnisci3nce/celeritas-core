/**
 * @file maths_types.h
 * @author Omniscient
 * @brief Maths types
 * @date 2024-02-24
 * @copyright Copyright (c) 2024
 */
#pragma once

#include "defines.h"

// --- Constants
#define PI 3.14159265358979323846
#define HALF_PI 1.57079632679489661923
#define TAU (2.0 * PI)

// --- Helpers
#define deg_to_rad(x) (x * 3.14 / 180.0)
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

// --- Types

/** @brief 2D Vector */
typedef struct vec2 {
  f32 x, y;
} vec2;

/** @brief 3D Vector */
typedef struct vec3 {
  f32 x, y, z;
} vec3;

/** @brief 4D Vector */
typedef struct vec4 {
  f32 x, y, z, w;
} vec4;

/** @brief Quaternion */
typedef vec4 quat;

/** @brief 4x4 Matrix */
typedef struct mat4 {
  // TODO: use this format for more readable code: vec4 x_axis, y_axis, z_axis, w_axis;
  f32 data[16];
} mat4;

/** @brief Three dimensional bounding box */
typedef struct bbox_3d {
  vec3 min;  // minimum point of the box
  vec3 max;  // maximum point of the box
} bbox_3d;

/** @brief 3D Axis-aligned bounding box */
typedef bbox_3d aabb_3d;

/** @brief 3D affine transformation */
typedef struct transform {
  vec3 position;
  quat rotation;
  f32 scale;
  bool is_dirty;
} transform;

typedef struct vec4i {
  i32 x, y, z, w;
} vec4i;

typedef struct vec4u {
  u32 x, y, z, w;
} vec4u;

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

typedef struct u32x2 {
  u32 x;
  u32 y;
} u32x2;
#define u32x2(x, y) ((u32x3){ x, y })

// Type aliass

typedef struct vec2 f32x2;
#define f32x2(x, y) ((f32x2){ x, y })

typedef struct vec3 f32x3;
#define f32x3(x, y, z) ((f32x3){ x, y, z })