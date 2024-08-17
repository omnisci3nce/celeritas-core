#include "pbr.h"
#include "animation.h"
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
#include "render_types.h"
#include "shader_layouts.h"


void PBR_Init(PBR_Storage* storage) {
  INFO("PBR shaders init");
  storage->pbr_pass = PBR_RPassCreate();
  PBR_PipelinesCreate(storage, storage->pbr_pass);
}

GPU_Renderpass* PBR_RPassCreate() {
  GPU_RenderpassDesc desc = { .default_framebuffer = true };
  return GPU_Renderpass_Create(desc);
}

void PBR_PipelinesCreate(PBR_Storage* storage, GPU_Renderpass* rpass) {
  // Common shader bindings
  ShaderDataLayout camera_data = Binding_Camera_GetLayout(NULL);
  ShaderDataLayout model_data = Binding_Model_GetLayout(NULL);
  ShaderDataLayout material_data = PBRMaterial_GetLayout(NULL);
  ShaderDataLayout lights_data = Binding_Lights_GetLayout(NULL);

  // Static
  {
    const char* vert_path = "assets/shaders/static_geometry.vert";
    const char* frag_path = "assets/shaders/pbr_textured.frag";
    char* vert_shader = string_from_file(vert_path);
    char* frag_shader = string_from_file(frag_path);

    GraphicsPipelineDesc desc = {
      .debug_name = "PBR (Static) Pipeline",
      .vertex_desc = static_3d_vertex_description(),
      .data_layouts = { camera_data, model_data, material_data, lights_data },
      .data_layouts_count = 4,
      .vs = { .debug_name = "PBR (textured) Vertex Shader",
              .filepath = str8(vert_path),
              .code = vert_shader },
      .fs = { .debug_name = "PBR (textured) Fragment Shader",
              .filepath = str8(frag_path),
              .code = frag_shader },
      .depth_test = true,
      .wireframe = true,
    };
    storage->pbr_static_pipeline = GPU_GraphicsPipeline_Create(desc, rpass);
  }

  // Skinned
  {
    const char* vert_path = "assets/shaders/skinned_geometry.vert";
    const char* frag_path = "assets/shaders/pbr_textured.frag";
    char* vert_shader = string_from_file(vert_path);
    char* frag_shader = string_from_file(frag_path);

    ShaderDataLayout anim_uniform = AnimData_GetLayout(NULL);

    VertexDescription vertex_desc = { .debug_label = "Skinned vertices",
                                      .use_full_vertex_size = true };
    VertexDesc_AddAttr(&vertex_desc, "inPosition", ATTR_F32x3);
    VertexDesc_AddAttr(&vertex_desc, "inNormal", ATTR_F32x3);
    VertexDesc_AddAttr(&vertex_desc, "inTexCoords", ATTR_F32x2);
    VertexDesc_AddAttr(&vertex_desc, "inBoneIndices", ATTR_I32x4);
    VertexDesc_AddAttr(&vertex_desc, "inWeights", ATTR_F32x4);

    GraphicsPipelineDesc desc = {
      .debug_name = "PBR (Skinned) Pipeline",
      .vertex_desc = vertex_desc,
      .data_layouts = { camera_data, model_data, material_data, lights_data, anim_uniform },
      .data_layouts_count = 5,
      .vs = { .debug_name = "PBR (textured) Vertex Shader",
              .filepath = str8(vert_path),
              .code = vert_shader },
      .fs = { .debug_name = "PBR (textured) Fragment Shader",
              .filepath = str8(frag_path),
              .code = frag_shader },
      .depth_test = true,
      .wireframe = true,
    };
    storage->pbr_skinned_pipeline = GPU_GraphicsPipeline_Create(desc, rpass);
  }
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

  // TEMP: only do skinned
  GPU_EncodeBindPipeline(enc, storage->pbr_skinned_pipeline);

  // Feed shader data
  Mat4 view, proj;
  u32x2 dimensions = GPU_Swapchain_GetDimensions();
  Camera_ViewProj(&camera, (f32)dimensions.x, (f32)dimensions.y, &view, &proj);
  Binding_Camera camera_data = { .view = view,
                                 .projection = proj,
                                 .viewPos = vec4(camera.position.x, camera.position.y,
                                                 camera.position.z, 1.0) };
  GPU_EncodeBindShaderData(enc, 0, Binding_Camera_GetLayout(&camera_data));

  Vec3 light_color = vec3(300.0, 300.0, 300.0);
  Binding_Lights
      lights_data = { .pointLights = {
                          // FIXME: add lights to our RenderScene structure. for now these are
                          // hardcoded
                          (pbr_point_light){ .pos = vec3(0.0, 6.0, 6.0), .color = light_color },
                          (pbr_point_light){ .pos = vec3(-10, 10, 10), .color = light_color },
                          (pbr_point_light){ .pos = vec3(10, -10, 10), .color = light_color },
                          (pbr_point_light){ .pos = vec3(-10, -10, 10), .color = light_color },
                      } };
  GPU_EncodeBindShaderData(enc, 3, Binding_Lights_GetLayout(&lights_data));

  // TODO: Add shadowmap texture to uniforms
  Mesh_pool* mesh_pool = Render_GetMeshPool();
  Material_pool* material_pool = Render_GetMaterialPool();

  for (size_t ent_i = 0; ent_i < entity_count; ent_i++) {
    RenderEnt renderable = entities[ent_i];
    Mesh* mesh = Mesh_pool_get(mesh_pool, renderable.mesh);
    Material* mat = Material_pool_get(material_pool, renderable.material);

    // upload material data
    PBRMaterialUniforms material_data = { .mat = *mat };
    GPU_EncodeBindShaderData(enc, 2, PBRMaterial_GetLayout(&material_data));

    // upload model transform
    Binding_Model model_data = { .model = renderable.affine };
    GPU_EncodeBindShaderData(enc, 1, Binding_Model_GetLayout(&model_data));

    // Skinning matrices

    // 1. calculate matrices
    AnimDataUniform anim_data = { 0 };
    CASSERT(renderable.armature);
    Armature* skeleton = renderable.armature;
    // Skip the first one as we assume its root for this test
    for (int j_i = 1; j_i < skeleton->joints->len; j_i++) {
        Joint* j = &skeleton->joints->data[j_i];
        j->local_transform = transform_to_mat(&j->transform_components);
        Mat4 m = mat4_mult(j->local_transform, j->inverse_bind_matrix);
        Joint* p = &skeleton->joints->data[j->parent];
        j->local_transform = mat4_mult(j->local_transform, p->local_transform);
        printf("Quat %f \n", j->transform_components.rotation.z);
    }

    // 2. bind and upload
    for (int j_i = 1; j_i < skeleton->joints->len; j_i++) {
      anim_data.bone_matrices[j_i] = skeleton->joints->data[j_i].local_transform;
    }
    GPU_EncodeBindShaderData(enc, 3, AnimData_GetLayout(&anim_data));

    // set buffers
    GPU_EncodeSetVertexBuffer(enc, mesh->vertex_buffer);
    GPU_EncodeSetIndexBuffer(enc, mesh->index_buffer);
    // draw
    GPU_EncodeDrawIndexedTris(enc, mesh->geometry.index_count);
  }

  GPU_CmdEncoder_EndRender(enc);
}

void PBRMaterial_BindData(ShaderDataLayout* layout, const void* data) {
  PBRMaterialUniforms* d = (PBRMaterialUniforms*)data;
  CASSERT(data);
  CASSERT(layout->binding_count == 5);

  TextureHandle white1x1 = Render_GetWhiteTexture();
  if (d->mat.albedo_map.raw != INVALID_TEX_HANDLE.raw) {
    layout->bindings[0].data.texture.handle = d->mat.albedo_map;
  } else {
    layout->bindings[0].data.texture.handle = white1x1;
  }
  // TODO .. the rest
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
