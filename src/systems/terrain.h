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
#include "ral_types.h"
#include "render.h"
#include "str.h"

typedef struct Heightmap {
  Str8 filepath;
  u32x2 pixel_dimensions;
  void* image_data;
  u32 num_channels;
  bool is_uploaded;
} Heightmap;

 typedef struct Terrain_Storage {
  // arena terrain_allocator;
  u32x2 grid_dimensions;
  f32 grid_scale;
  u32 num_vertices;
  Heightmap heightmap;  // NULL = no heightmap
  GPU_Renderpass* hmap_renderpass;
  GPU_Pipeline* hmap_pipeline;

  bool hmap_loaded;
  BufferHandle vertex_buffer;
  BufferHandle index_buffer;
  u32 indices_count;
} Terrain_Storage;

// --- Public API
PUB bool Terrain_Init(Terrain_Storage* storage);
PUB void Terrain_Shutdown(Terrain_Storage* storage);
PUB void Terrain_Draw(
    Terrain_Storage* storage);  // NOTE: For now it renders directly to main framebuffer

/** @brief Sets the active heightmap to be rendered and collided against. */
PUB void Terrain_LoadHeightmap(Terrain_Storage* storage, Heightmap hmap, f32 grid_scale, bool free_on_upload);
PUB Heightmap Heightmap_FromImage(Str8 filepath);
PUB Heightmap Heightmap_FromPerlin(/* TODO: perlin noise generation parameters */);

PUB bool Terrain_IsActive();  // checks whether we have a loaded heightmap and it's being rendered

// --- Internal

// TODO: void terrain_system_render_hmap(renderer* rend, terrain_state* state);

/** @brief Get the height (the Y component) for a vertex at a particular coordinate in the heightmap
 */
f32 Heightmap_HeightXZ(const Heightmap* hmap, u32 x, u32 z);

/** @brief Calculate the normal vector of a vertex at a particular coordinate in the heightmap */
Vec3 Heightmap_NormalXZ(const Heightmap* hmap, f32 x, f32 z);

// /** @brief Generate the `geometry_data` for a heightmap ready to be uploaded to the GPU */
// Geometry geo_heightmap(arena* a, Heightmap heightmap);

ShaderDataLayout TerrainUniforms_GetLayout(void* data);