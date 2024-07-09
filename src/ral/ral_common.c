#include "ral_common.h"
#include "ral_impl.h"

void backend_pools_init(arena* a, gpu_backend_pools* backend_pools) {
  pipeline_layout_pool pipeline_layout_pool =
      pipeline_layout_pool_create(a, MAX_PIPELINES, sizeof(gpu_pipeline_layout));
  backend_pools->pipeline_layouts = pipeline_layout_pool;
  pipeline_pool pipeline_pool = pipeline_pool_create(a, MAX_PIPELINES, sizeof(gpu_pipeline));
  backend_pools->pipelines = pipeline_pool;
  renderpass_pool rpass_pool = renderpass_pool_create(a, MAX_RENDERPASSES, sizeof(gpu_renderpass));
  backend_pools->renderpasses = rpass_pool;
}

void resource_pools_init(arena* a, struct resource_pools* res_pools) {
  buffer_pool buf_pool = buffer_pool_create(a, MAX_BUFFERS, sizeof(gpu_buffer));
  res_pools->buffers = buf_pool;
  texture_pool tex_pool = texture_pool_create(a, MAX_TEXTURES, sizeof(gpu_texture));
  res_pools->textures = tex_pool;
}