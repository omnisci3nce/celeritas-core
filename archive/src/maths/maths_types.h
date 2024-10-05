/**
 * @file maths_types.h
 * @author Omniscient
 * @brief Maths types
 * @date 2024-02-24
 * @copyright Copyright (c) 2024
 */
#pragma once

#include "defines.h"

// --- Types

/** @brief 2D Vector */
typedef struct Vec2 {
  f32 x, y;
} Vec2;

/** @brief 4x4 Matrix */
typedef struct Mat4 {
  // TODO: use this format for more readable code: vec4 x_axis, y_axis, z_axis, w_axis;
  f32 data[16];
} Mat4;

/** @brief Three dimensional bounding box */
typedef struct Bbox_3D {
  Vec3 min;  // minimum point of the box
  Vec3 max;  // maximum point of the box
} Bbox_3D;

/** @brief 3D Axis-aligned bounding box */
typedef Bbox_3D Aabb_3D;

/** @brief 3D affine transformation */
typedef struct Transform {
  Vec3 position;
  Quat rotation;
  Vec3 scale;
  bool is_dirty;
} Transform;

typedef struct Vec4i {
  i32 x, y, z, w;
} Vec4i;

typedef struct Vec4u {
  u32 x, y, z, w;
} Vec4u;

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
#define u32x2(x, y) ((u32x2){ x, y })

// Type aliass

typedef struct Vec2 f32x2;
#define f32x2(x, y) ((f32x2){ x, y })

typedef struct Vec3 f32x3;
#define f32x3(x, y, z) ((f32x3){ x, y, z })
