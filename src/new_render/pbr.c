#include "pbr.h"
#include "file.h"
#include "log.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "shader_layouts.h"

void PBR_Init(PBR_Storage* storage) {
  INFO("PBR shaders init");
  storage->pbr_pass = PBR_RPassCreate();
  storage->pbr_pipeline = PBR_PipelineCreate(storage->pbr_pass);
}

GPU_Renderpass* PBR_RPassCreate() {
  GPU_RenderpassDesc desc = { .default_framebuffer = true };
  return GPU_Renderpass_Create(desc);
}

GPU_Pipeline* PBR_PipelineCreate(GPU_Renderpass* rpass) {
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  Str8 vert_path = str8("assets/shaders/pbr_textured.vert");
  Str8 frag_path = str8("assets/shaders/pbr_textured.frag");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  ShaderData camera_data = { .get_layout = &Binding_Camera_GetLayout };
  ShaderData model_data = { .get_layout = &Binding_Model_GetLayout };
  ShaderData material_data = { .get_layout = &PBRMaterial_GetLayout };

  GraphicsPipelineDesc desc = {
    .debug_name = "PBR Pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = {camera_data,model_data,material_data},
    .data_layouts_count = 3,
    .vs = { .debug_name = "PBR (textured) Vertex Shader",
            .filepath = vert_path,
            .code = vertex_shader.contents },
    .fs = { .debug_name = "PBR (textured) Fragment Shader",
            .filepath = frag_path,
            .code = fragment_shader.contents,
            },
    .depth_test = true,
    .wireframe = false,
  };
  return GPU_GraphicsPipeline_Create(desc, rpass);
}

ShaderDataLayout PBRMaterial_GetLayout(void* data) {
  PBRMaterialUniforms* d = (PBRMaterialUniforms*)data;
  bool has_data = data != NULL;

  ShaderBinding b1 = {
    .label = "albedoMap",
    .kind = BINDING_TEXTURE,
  };
  ShaderBinding b2 = {
    .label = "metallicRoughnessMap",
    .kind = BINDING_TEXTURE,
  };
  ShaderBinding b3 = {
    .label = "aoMap",
    .kind = BINDING_TEXTURE,
  };
  ShaderBinding b4 = {
    .label = "normalMap",
    .kind = BINDING_TEXTURE,
  };

  if (has_data) {
    b1.data.texture.handle = d->mat.pbr_albedo_map;
    b2.data.texture.handle = d->mat.pbr_metallic_map;
    b3.data.texture.handle = d->mat.pbr_ao_map;
    b4.data.texture.handle = d->mat.pbr_normal_map;
  }

  return (ShaderDataLayout){ .bindings = { b1, b2, b3, b4 }, .binding_count = 4 };
}