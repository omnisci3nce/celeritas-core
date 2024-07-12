#include "ral_common.h"
#include "ral_impl.h"

void backend_pools_init(arena* a, GPU_BackendPools* backend_pools) {
  PipelineLayout_pool pipeline_layout_pool =
      PipelineLayout_pool_create(a, MAX_PIPELINES, sizeof(GPU_PipelineLayout));
  backend_pools->pipeline_layouts = pipeline_layout_pool;
  Pipeline_pool pipeline_pool = Pipeline_pool_create(a, MAX_PIPELINES, sizeof(GPU_Pipeline));
  backend_pools->pipelines = pipeline_pool;
  Renderpass_pool rpass_pool = Renderpass_pool_create(a, MAX_RENDERPASSES, sizeof(GPU_Renderpass));
  backend_pools->renderpasses = rpass_pool;
}

void resource_pools_init(arena* a, struct ResourcePools* res_pools) {
  Buffer_pool buf_pool = Buffer_pool_create(a, MAX_BUFFERS, sizeof(GPU_Buffer));
  res_pools->buffers = buf_pool;
  Texture_pool tex_pool = Texture_pool_create(a, MAX_TEXTURES, sizeof(GPU_Texture));
  res_pools->textures = tex_pool;
}
