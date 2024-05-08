#pragma once

#include <assert.h>
#include <stdlib.h>
#include "core.h"
#include "maths_types.h"
#include "render_types.h"

geometry_data geo_create_plane(f32x2 extents);
geometry_data geo_create_cuboid(f32x3 extents);
geometry_data geo_create_cylinder(f32 radius, f32 height, u32 resolution);
geometry_data geo_create_uvsphere(f32 radius, f32 north_south_lines, f32 east_west_lines);
geometry_data geo_create_icosphere(f32 radius, f32 n_subdivisions);