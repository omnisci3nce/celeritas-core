#pragma once
#include "defines.h"
#include "mem.h"
#include "ral_types.h"
#include "ral_impl.h"

CORE_DEFINE_HANDLE(BufferHandle);
CORE_DEFINE_HANDLE(TextureHandle);
CORE_DEFINE_HANDLE(SamplerHandle);
CORE_DEFINE_HANDLE(ShaderHandle);

CORE_DEFINE_HANDLE(pipeline_layout_handle);
CORE_DEFINE_HANDLE(pipeline_handle);
CORE_DEFINE_HANDLE(renderpass_handle);

#define MAX_SHADER_DATA_LAYOUTS 8
#define MAX_BUFFERS 256
#define MAX_TEXTURES 256
#define MAX_PIPELINES 128
#define MAX_RENDERPASSES 128

TYPED_POOL(GPU_Buffer, Buffer);
TYPED_POOL(gpu_texture, texture);
TYPED_POOL(gpu_pipeline_layout, pipeline_layout);
TYPED_POOL(gpu_pipeline, pipeline);
TYPED_POOL(gpu_renderpass, renderpass);

// --- Handy macros
#define BUFFER_GET(h) (buffer_pool_get(&context.resource_pools->buffers, h))
#define TEXTURE_GET(h) (texture_pool_get(&context.resource_pools->textures, h))

// --- Pools
typedef struct gpu_backend_pools {
  pipeline_pool pipelines;
  pipeline_layout_pool pipeline_layouts;
  renderpass_pool renderpasses;
} gpu_backend_pools;
void backend_pools_init(arena* a, gpu_backend_pools* backend_pools);

struct resource_pools {
  buffer_pool buffers;
  texture_pool textures;
};
void resource_pools_init(arena* a, struct resource_pools* res_pools);

// vertex_description static_3d_vertex_description();
