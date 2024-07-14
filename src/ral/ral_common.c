#include "ral_common.h"
#include "ral_impl.h"

void BackendPools_Init(arena* a, GPU_BackendPools* backend_pools) {
  PipelineLayout_pool pipeline_layout_pool =
      PipelineLayout_pool_create(a, MAX_PIPELINES, sizeof(GPU_PipelineLayout));
  backend_pools->pipeline_layouts = pipeline_layout_pool;
  Pipeline_pool pipeline_pool = Pipeline_pool_create(a, MAX_PIPELINES, sizeof(GPU_Pipeline));
  backend_pools->pipelines = pipeline_pool;
  Renderpass_pool rpass_pool = Renderpass_pool_create(a, MAX_RENDERPASSES, sizeof(GPU_Renderpass));
  backend_pools->renderpasses = rpass_pool;
}

void ResourcePools_Init(arena* a, struct ResourcePools* res_pools) {
  Buffer_pool buf_pool = Buffer_pool_create(a, MAX_BUFFERS, sizeof(GPU_Buffer));
  res_pools->buffers = buf_pool;
  Texture_pool tex_pool = Texture_pool_create(a, MAX_TEXTURES, sizeof(GPU_Texture));
  res_pools->textures = tex_pool;
}

VertexDescription static_3d_vertex_description() {
    VertexDescription builder = { .debug_label = "Standard static 3d vertex format" };
    VertexDesc_AddAttr(&builder, "inPosition", ATTR_F32x3);
    VertexDesc_AddAttr(&builder, "inNormal", ATTR_F32x3);
    VertexDesc_AddAttr(&builder, "inTexCoords", ATTR_F32x2);
    builder.use_full_vertex_size = true;
    return builder;
}

void VertexDesc_AddAttr(VertexDescription* builder, const char* name, VertexAttribType type) {
    u32 i = builder->attributes_count;

    size_t size = VertexAttribSize(type);
    builder->attributes[i] = type;
    builder->stride += size;
    builder->attr_names[i] = name;

    builder->attributes_count++;
}

size_t VertexAttribSize(VertexAttribType attr) {
  switch (attr) {
    case ATTR_F32:
    case ATTR_U32:
    case ATTR_I32:
      return 4;
    case ATTR_F32x2:
    case ATTR_U32x2:
    case ATTR_I32x2:
      return 8;
    case ATTR_F32x3:
    case ATTR_U32x3:
    case ATTR_I32x3:
      return 12;
    case ATTR_F32x4:
    case ATTR_U32x4:
    case ATTR_I32x4:
      return 16;
      break;
  }
}
