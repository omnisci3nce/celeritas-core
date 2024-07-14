/**
 * @brief Common functions that don't actually depend on the specific backend
*/
#pragma once
#include "defines.h"
#include "buf.h"
#include "mem.h"
#include "ral_types.h"
#include "ral_impl.h"

TYPED_POOL(GPU_Buffer, Buffer);
TYPED_POOL(GPU_Texture, Texture);
TYPED_POOL(GPU_PipelineLayout, PipelineLayout);
TYPED_POOL(GPU_Pipeline, Pipeline);
TYPED_POOL(GPU_Renderpass, Renderpass);

// --- Handy macros
#define BUFFER_GET(h) (buffer_pool_get(&context.resource_pools->buffers, h))
#define TEXTURE_GET(h) (texture_pool_get(&context.resource_pools->textures, h))

// --- Pools
typedef struct GPU_BackendPools {
  Pipeline_pool pipelines;
  PipelineLayout_pool pipeline_layouts;
  Renderpass_pool renderpasses;
} GPU_BackendPools;
void BackendPools_Init(arena* a, GPU_BackendPools* backend_pools);

struct ResourcePools {
  Buffer_pool buffers;
  Texture_pool textures;
};
typedef struct ResourcePools ResourcePools;
void ResourcePools_Init(arena* a, struct ResourcePools* res_pools);

// --- Vertex formats
VertexDescription static_3d_vertex_description();

void VertexDesc_AddAttr(VertexDescription* builder, const char* name, VertexAttribType type);

size_t VertexAttribSize(VertexAttribType attr);
