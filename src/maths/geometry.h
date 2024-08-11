/**
 * @file geometry.h
 * @author your name (you@domain.com)
 * @brief Shapes and intersections between them
 * @version 0.1
 * @date 2024-02-24
 *
 * @copyright Copyright (c) 2024
 */
#pragma once

#include "maths.h"

// typedef struct line_3d {
//   vec3 start, end;
// } line_3d;

// typedef struct plane {
//   vec3 normal;
// } plane;

typedef struct Cuboid {
  Vec3 half_extents;
} Cuboid;

typedef struct Sphere {
  f32 radius;
} Sphere;

// typedef struct cylinder {
//   f32 radius;
//   f32 half_height;
// } cylinder;

// typedef struct cone {
//   f32 radius;
//   f32 half_height;
// } cone;

// TODO:
// capsule
// torus
// ray
// frustum
// conical frustum
// wedge

// 2d...
// line
// circle
