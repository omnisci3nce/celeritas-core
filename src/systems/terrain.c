/**
 * @brief
 */
#include "terrain.h"
#include "ral.h"

struct Terrain_Storage {
  arena terrain_allocator;
  Heightmap* heightmap;  // NULL = no heightmap
  GPU_Renderpass* hmap_renderpass;
  GPU_Pipeline* hmap_pipeline;
};

bool Terrain_Init(Terrain_Storage* storage) {
    return true;
}

void Terrain_Shutdown(Terrain_Storage* storage);

/* bool terrain_system_init(terrain_state* state) { */
/*   gpu_renderpass_desc rpass_desc = { */
/*     .default_framebuffer = true, */
/*   }; */
/*   struct graphics_pipeline_desc pipeline_desc = { */

/*   }; */

/*   state->hmap_renderpass = gpu_renderpass_create(&rpass_desc); */
/*   state->hmap_pipeline = gpu_graphics_pipeline_create(pipeline_desc); */
/* } */
