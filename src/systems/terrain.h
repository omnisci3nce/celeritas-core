/**
 * @file terrain.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-27
 *
 * @copyright Copyright (c) 2024
 *
 */

/*
Future:
 - Chunked terrain
 - Dynamic LOD
*/

#include "cleanroom/types.h"
#include "defines.h"
#include "maths_types.h"
#include "mem.h"

typedef struct terrain_state {
} terrain_state;

typedef struct heightmap {
  u32x2 size;
  void* image_data;
} heightmap;

bool terrain_system_init(terrain_state* state);
void terrain_system_shutdown(terrain_state* state);
void terrain_system_render_hmap(renderer* rend, terrain_state* state);

heightmap heightmap_from_image(const char* filepath);
heightmap heightmap_from_perlin(/* TODO: perlin noise generation parameters */);

/** @brief Get the height (the Y component) for a vertex at a particular coordinate in the heightmap
 */
f32 heightmap_height_at_xz(heightmap* hmap, f32 x, f32 z);

/** @brief Calculate the normal vector of a vertex at a particular coordinate in the heightmap */
vec3 heightmap_normal_at_xz(heightmap* hmap, f32 x, f32 z);

/** @brief Generate the `geometry_data` for a heightmap ready to be uploaded to the GPU */
geometry_data geo_heightmap(arena* a, heightmap heightmap);