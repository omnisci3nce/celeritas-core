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



/** @brief Three dimensional bounding box */
typedef struct Bbox_3D {
  Vec3 min;  // minimum point of the box
  Vec3 max;  // maximum point of the box
} Bbox_3D;

/** @brief 3D Axis-aligned bounding box */
typedef Bbox_3D Aabb_3D;



typedef struct Vec4i {
  i32 x, y, z, w;
} Vec4i;

typedef struct Vec4u {
  u32 x, y, z, w;
} Vec4u;
