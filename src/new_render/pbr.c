#include "pbr.h"
#include "camera.h"
#include "core.h"
#include "file.h"
#include "log.h"
#include "maths.h"
#include "mem.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render.h"
#include "render_scene.h"
#include "render_types.h"
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

  const char* vert_path = "assets/shaders/pbr_textured.vert";
  const char* frag_path = "assets/shaders/pbr_textured.frag";
  // Str8 vert_path = str8("assets/shaders/pbr_textured.vert");
  // Str8 frag_path = str8("assets/shaders/pbr_textured.frag");
  // str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  // str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  // if (!vertex_shader.has_value || !fragment_shader.has_value) {
  //   ERROR_EXIT("Failed to load shaders from disk")
  // }
  char* vert_shader = string_from_file(vert_path);
  char* frag_shader = string_from_file(frag_path);

  ShaderData camera_data = { .get_layout = &Binding_Camera_GetLayout };
  ShaderData model_data = { .get_layout = &Binding_Model_GetLayout };
  ShaderData material_data = { .get_layout = &PBRMaterial_GetLayout };
  ShaderData lights_data = { .get_layout = &Binding_Lights_GetLayout };

  GraphicsPipelineDesc desc = {
    .debug_name = "PBR Pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = {camera_data,model_data,material_data, lights_data },
    .data_layouts_count = 4,
    .vs = { .debug_name = "PBR (textured) Vertex Shader",
            .filepath = str8(vert_path),
            // .code = vertex_shader.contents
            .code = vert_shader
             },
    .fs = { .debug_name = "PBR (textured) Fragment Shader",
            .filepath = str8(frag_path),
            .code = frag_shader
            // .code = fragment_shader.contents,
            },
    .depth_test = true,
    .wireframe = false,
  };
  return GPU_GraphicsPipeline_Create(desc, rpass);
}

void PBR_Execute(PBR_Storage* storage, Camera camera, TextureHandle shadowmap_tex,
                 RenderEnt* entities, size_t entity_count) {
  // 1. set up our pipeline
  // 2. upload constant data (camera, lights)
  // 3. draw each entity
  //  - upload material data -> in the future we will sort & batch by material
  //  - upload model transform
  //  - emit draw call

  GPU_CmdEncoder* enc = GPU_GetDefaultEncoder();
  GPU_CmdEncoder_BeginRender(enc, storage->pbr_pass);
  GPU_EncodeBindPipeline(enc, storage->pbr_pipeline);

  // Feed shader data
  Mat4 view, proj;
  u32x2 dimensions = GPU_Swapchain_GetDimensions();
  Camera_ViewProj(&camera, (f32)dimensions.x, (f32)dimensions.y, &view, &proj);
  Binding_Camera camera_data = { .view = view,
                                 .projection = proj,
                                 .viewPos = vec4(camera.position.x, camera.position.y,
                                                 camera.position.z, 1.0) };
  GPU_EncodeBindShaderData(
      enc, 0, (ShaderData){ .data = &camera_data, .get_layout = &Binding_Camera_GetLayout });

  Vec3 light_color = vec3(300.0, 300.0, 300.0);
  Binding_Lights
      lights_data = { .pointLights = {
                          // FIXME: add lights to our RenderScene structure. for now these are
                          // hardcoded
                          (pbr_point_light){ .pos = vec3(10, 10, 10), .color = light_color },
                          (pbr_point_light){ .pos = vec3(-10, 10, 10), .color = light_color },
                          (pbr_point_light){ .pos = vec3(10, -10, 10), .color = light_color },
                          (pbr_point_light){ .pos = vec3(-10, -10, 10), .color = light_color },
                      } };
  GPU_EncodeBindShaderData(
      enc, 3, (ShaderData){ .data = &lights_data, .get_layout = &Binding_Lights_GetLayout });

  // TODO: Add shadowmap texture to uniforms

  for (size_t ent_i = 0; ent_i < entity_count; ent_i++) {
    RenderEnt renderable = entities[ent_i];

    // upload material data
    PBRMaterialUniforms material_data = { .mat = *renderable.material };
    GPU_EncodeBindShaderData(
        enc, 2, (ShaderData){ .data = &material_data, .get_layout = PBRMaterial_GetLayout });

    // upload model transform
    Binding_Model model_data = { .model = renderable.affine };
    GPU_EncodeBindShaderData(
        enc, 1, (ShaderData){ .data = &model_data, .get_layout = &Binding_Model_GetLayout });

    // set buffers
    GPU_EncodeSetVertexBuffer(enc, renderable.mesh->vertex_buffer);
    GPU_EncodeSetIndexBuffer(enc, renderable.mesh->index_buffer);
    // draw
    GPU_EncodeDrawIndexed(enc, renderable.mesh->geometry.indices->len);
  }

  GPU_CmdEncoder_EndRender(enc);
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
  ShaderBinding b5 = { .label = "PBR_Params",
                       .kind = BINDING_BYTES,
                       .data.bytes.size = sizeof(PBR_Params) };

  if (has_data) {
    TextureHandle white1x1 = Render_GetWhiteTexture();
    if (d->mat.albedo_map.raw != INVALID_TEX_HANDLE.raw) {
      b1.data.texture.handle = d->mat.albedo_map;
    } else {
      b1.data.texture.handle = white1x1;
    }

    if (d->mat.metallic_roughness_map.raw != INVALID_TEX_HANDLE.raw) {
      b2.data.texture.handle = d->mat.metallic_roughness_map;
    } else {
      b2.data.texture.handle = white1x1;
    }

    if (d->mat.ambient_occlusion_map.raw != INVALID_TEX_HANDLE.raw) {
      b3.data.texture.handle = d->mat.ambient_occlusion_map;
    } else {
      b3.data.texture.handle = white1x1;
    }

    if (d->mat.normal_map.raw != INVALID_TEX_HANDLE.raw) {
      b4.data.texture.handle = d->mat.normal_map;
    } else {
      b4.data.texture.handle = white1x1;
    }

    arena* frame = Render_GetFrameArena();
    PBR_Params* params = arena_alloc(frame, sizeof(PBR_Params));
    params->albedo = d->mat.base_colour;
    params->metallic = d->mat.metallic;
    params->roughness = d->mat.roughness;
    params->ambient_occlusion = d->mat.ambient_occlusion;
    b5.data.bytes.data = params;
  }

  return (ShaderDataLayout){ .bindings = { b1, b2, b3, b4, b5 }, .binding_count = 5 };
}

Material PBRMaterialDefault() {
  return (Material){ .name = "Standard Material",
                     .kind = MAT_PBR,
                     .base_colour = vec3(1.0, 1.0, 1.0),
                     .metallic = 0.0,
                     .roughness = 0.5,
                     .ambient_occlusion = 0.0,
                     .albedo_map = INVALID_TEX_HANDLE,
                     .metallic_roughness_map = INVALID_TEX_HANDLE,
                     .normal_map = INVALID_TEX_HANDLE,
                     .ambient_occlusion_map = INVALID_TEX_HANDLE };
}