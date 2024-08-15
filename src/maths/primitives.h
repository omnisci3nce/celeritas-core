#pragma once

#include <assert.h>
#include <stdlib.h>
#include "core.h"
#include "maths_types.h"
#include "render_types.h"

Geometry Geo_CreatePlane(f32x2 extents, u32 tiling_u, u32 tiling_v);
Geometry Geo_CreateCuboid(f32x3 extents);
Geometry Geo_CreateCylinder(f32 radius, f32 height, u32 resolution);
Geometry Geo_CreateCone(f32 radius, f32 height, u32 resolution);
Geometry Geo_CreateUVsphere(f32 radius, u32 north_south_lines, u32 east_west_lines);
Geometry Geo_CreateIcosphere(f32 radius, f32 n_subdivisions);

static const Vec3 BACK_BOT_LEFT = (Vec3){ 0, 0, 0 };
static const Vec3 BACK_BOT_RIGHT = (Vec3){ 1, 0, 0 };
static const Vec3 BACK_TOP_LEFT = (Vec3){ 0, 1, 0 };
static const Vec3 BACK_TOP_RIGHT = (Vec3){ 1, 1, 0 };
static const Vec3 FRONT_BOT_LEFT = (Vec3){ 0, 0, 1 };
static const Vec3 FRONT_BOT_RIGHT = (Vec3){ 1, 0, 1 };
static const Vec3 FRONT_TOP_LEFT = (Vec3){ 0, 1, 1 };
static const Vec3 FRONT_TOP_RIGHT = (Vec3){ 1, 1, 1 };

#define VERT_3D(arr, pos, norm, uv)                                                    \
  {                                                                                    \
    Vertex v = { .static_3d = { .position = pos, .normal = norm, .tex_coords = uv } }; \
    Vertex_darray_push(arr, v);                                                        \
  }