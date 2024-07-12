#pragma once

#include <assert.h>
#include <stdlib.h>
#include "core.h"
#include "maths_types.h"
#include "render_types.h"

Geometry Geo_CreatePlane(f32x2 extents);
Geometry Geo_CreateCuboid(f32x3 extents);
Geometry Geo_CreateCylinder(f32 radius, f32 height, u32 resolution);
Geometry Geo_CreateUVsphere(f32 radius, u32 north_south_lines, u32 east_west_lines);
Geometry Geo_CreateIcosphere(f32 radius, f32 n_subdivisions);
