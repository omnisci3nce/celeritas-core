/**
 * @brief Common functions that don't actually depend on the specific backend
 */
#pragma once
#include "buf.h"
#include "defines.h"
#include "mem.h"
#include "ral_types.h"
// #include "ral_impl.h"

// Concrete implementation
#if defined(CEL_REND_BACKEND_OPENGL)
#include "backend_opengl.h"
#endif

TYPED_POOL(GPU_Buffer, Buffer);
TYPED_POOL(GPU_Texture, Texture);
TYPED_POOL(GPU_PipelineLayout, PipelineLayout);
TYPED_POOL(GPU_Pipeline, Pipeline);
TYPED_POOL(GPU_Renderpass, Renderpass);

// --- Handy macros
#define BUFFER_GET(h) (Buffer_pool_get(&context.resource_pools->buffers, h))
#define TEXTURE_GET(h) (Texture_pool_get(&context.resource_pools->textures, h))

// --- Views
typedef struct GPU_BufferView {
  BufferHandle buf;
  size_t offset;
  size_t bytes;
} GPU_BufferView;

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

PUB GPU_Renderpass* GPU_GetDefaultRenderpass();  // returns a renderpass that draws directly to
                                                 // default framebuffer with default depth

// --
// window resize callback
void GPU_WindowResizedCallback(u32 x, u32 y);

// --- Vertex formats
VertexDescription static_3d_vertex_description();

void VertexDesc_AddAttr(VertexDescription* builder, const char* name, VertexAttribType type);
size_t VertexDesc_CalcStride(VertexDescription* desc);

size_t VertexAttribSize(VertexAttribType attr);
