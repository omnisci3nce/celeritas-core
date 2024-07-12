/**
 * @file terrain.h
 * @brief
 */

#pragma once

/*
Future:
 - Chunked terrain
 - Dynamic LOD
*/

#include "defines.h"
#include "maths_types.h"
#include "mem.h"
#include "ral.h"
#include "render.h"
#include "str.h"

typedef struct Heightmap {
  Str8 filepath;
  u32x2 size;
  void* image_data;
  bool is_uploaded;
} Heightmap;

typedef struct Terrain_Storage Terrain_Storage;

// --- Public API
PUB bool Terrain_Init(Terrain_Storage* storage);
PUB void Terrain_Shutdown(Terrain_Storage* storage);
PUB void Terrain_Run(Terrain_Storage* storage); // NOTE: For now it renders directly to main framebuffer

/** @brief Sets the active heightmap to be rendered and collided against. */
PUB Heightmap Terrain_LoadHeightmap(Heightmap hmap, bool free_on_upload);
PUB Heightmap Heightmap_FromImage(Str8 filepath);
PUB Heightmap Heightmap_FromPerlin(/* TODO: perlin noise generation parameters */);

PUB bool Terrain_IsActive(); // checks whether we have a loaded heightmap and it's being rendered

// --- Internal

// TODO: void terrain_system_render_hmap(renderer* rend, terrain_state* state);

/** @brief Get the height (the Y component) for a vertex at a particular coordinate in the heightmap
 */
f32 Heightmap_HeightXZ(Heightmap* hmap, f32 x, f32 z);

/** @brief Calculate the normal vector of a vertex at a particular coordinate in the heightmap */
Vec3 Heightmap_NormalXZ(Heightmap* hmap, f32 x, f32 z);

/** @brief Generate the `geometry_data` for a heightmap ready to be uploaded to the GPU */
Geometry geo_heightmap(arena* a, Heightmap heightmap);
