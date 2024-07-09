#include "ral.h"
#include "file.h"
#include "log.h"
#include "mem.h"
#include "str.h"

#if defined(CEL_REND_BACKEND_VULKAN)
#include "backend_vulkan.h"
#elif defined(CEL_REND_BACKEND_METAL)
#include "backend_metal.h"
#elif defined(CEL_REND_BACKEND_OPENGL)
#include "backend_opengl.h"
#endif

size_t vertex_attrib_size(vertex_attrib_type attr) {
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

void vertex_desc_add(vertex_description* builder, const char* name, vertex_attrib_type type) {
  u32 i = builder->attributes_count;

  size_t size = vertex_attrib_size(type);
  builder->attributes[i] = type;
  builder->stride += size;
  builder->attr_names[i] = name;

  builder->attributes_count++;
}

vertex_description static_3d_vertex_description() {
  vertex_description builder = { .debug_label = "Standard static 3d vertex format" };
  vertex_desc_add(&builder, "inPosition", ATTR_F32x3);
  vertex_desc_add(&builder, "inNormal", ATTR_F32x3);
  vertex_desc_add(&builder, "inTexCoords", ATTR_F32x2);
  builder.use_full_vertex_size = true;
  return builder;
}

void backend_pools_init(arena* a, gpu_backend_pools* backend_pools) {
  pipeline_layout_pool pipeline_layout_pool =
      pipeline_layout_pool_create(a, MAX_PIPELINES, sizeof(gpu_pipeline_layout));
  backend_pools->pipeline_layouts = pipeline_layout_pool;
  pipeline_pool pipeline_pool = pipeline_pool_create(a, MAX_PIPELINES, sizeof(gpu_pipeline));
  backend_pools->pipelines = pipeline_pool;
  renderpass_pool rpass_pool = renderpass_pool_create(a, MAX_RENDERPASSES, sizeof(gpu_renderpass));
  backend_pools->renderpasses = rpass_pool;

  // context.gpu_pools;
}

void resource_pools_init(arena* a, struct resource_pools* res_pools) {
  buffer_pool buf_pool = buffer_pool_create(a, MAX_BUFFERS, sizeof(gpu_buffer));
  res_pools->buffers = buf_pool;
  texture_pool tex_pool = texture_pool_create(a, MAX_TEXTURES, sizeof(gpu_texture));
  res_pools->textures = tex_pool;

  // context.resource_pools = res_pools;
}

void print_shader_binding(shader_binding b) {
  printf("Binding name: %s type %s vis %d stores data %d\n", b.label,
         shader_binding_type_name[b.type], b.vis, b.stores_data);
}

shader_desc shader_quick_load(const char* filepath) {
  arena a = arena_create(malloc(1024 * 1024), 1024 * 1024);
  str8 path = str8_cstr_view(filepath);
  str8_opt shader = str8_from_file(&a, path);
  if (!shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk");
  }

  return (shader_desc){
    .debug_name = filepath,
    .code = shader.contents,
    .filepath = path,
    .is_spirv = true,
  };
}
