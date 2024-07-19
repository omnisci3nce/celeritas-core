

#include "render_frame.h"
#include <assert.h>
#include "logos/threadpool.h"
#include "mem.h"
#include "render.h"


Cull_Result Frame_Cull(Renderer* ren, RenderEnt *entities, size_t entity_count, Camera *camera) {
  // TODO: u32 chunk_count = Tpool_GetNumWorkers();

  arena* frame_arena = GetRenderFrameArena(ren);

  Cull_Result result = {0};
  result.visible_ent_indices = arena_alloc(frame_arena, sizeof(u32) * entity_count); // make space for if all ents are visible

  assert((result.n_visible_objects + result.n_culled_objects == entity_count));
  return result;
}