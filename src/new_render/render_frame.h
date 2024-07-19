#pragma once
#include "camera.h"
#include "defines.h"
#include "ral_types.h"
#include "render_types.h"

// Frame lifecycle on CPU

// 1. extract
// 2. culling
// 3. render
// 4. dispatch (combined with render for now)

typedef struct Cull_Result {
  u64 n_visible_objects;
  u64 n_culled_objects;
  u32* visible_ent_indices; // allocated on frame arena
  size_t index_count;
} Cull_Result;

// everything that can be in the world, knows how to extract rendering data
typedef void (*ExtractRenderData)(void* world_data);

typedef struct Renderer Renderer;

/** @brief Produces a smaller set of only those meshes visible in the camera frustum on the CPU */
Cull_Result Frame_Cull(Renderer* ren, RenderEnt* entities, size_t entity_count, Camera *camera);