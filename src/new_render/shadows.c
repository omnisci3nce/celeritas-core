#include "shadows.h"
#include <string.h>
#include "file.h"
#include "glad/glad.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "primitives.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render.h"
#include "render_scene.h"
#include "render_types.h"
#include "str.h"

ShaderDataLayout ShadowUniforms_GetLayout(void* data) {
  ShadowUniforms* d = (ShadowUniforms*)data;
  bool has_data = data != NULL;

  ShaderBinding b1 = {
    .label = "ShadowUniforms",
    .kind = BINDING_BYTES,
    .vis = VISIBILITY_VERTEX,
    .data = { .bytes = { .size = sizeof(ShadowUniforms) } }
    // TODO: split this into two bindings so we can update model matrix independently
  };

  if (has_data) {
    b1.data.bytes.data = data;
  }

  return (ShaderDataLayout){ .binding_count = 1, .bindings = { b1 } };
}

ShaderDataLayout ShadowDebugQuad_GetLayout(void* data) {
  TextureHandle* handle = data;
  bool has_data = data != NULL;

  ShaderBinding b1 = {
    .label = "depthMap",
    .kind = BINDING_TEXTURE,
    .vis = VISIBILITY_FRAGMENT,
  };

  if (has_data) {
    b1.data.texture.handle = *handle;
  }

  return (ShaderDataLayout){ .binding_count = 1, .bindings = { b1 } };
}

void Shadow_Init(Shadow_Storage* storage, u32 shadowmap_width, u32 shadowmap_height) {
  memset(storage, 0, sizeof(Shadow_Storage));
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  TextureDesc depthmap_desc = { .extents = u32x2(shadowmap_width, shadowmap_height),
                                .format = TEXTURE_FORMAT_DEPTH_DEFAULT,
                                .tex_type = TEXTURE_TYPE_2D };
  DEBUG("Creating depth map texture for shadows");
  TextureHandle depthmap = GPU_TextureCreate(depthmap_desc, false, NULL);
  storage->depth_texture = depthmap;

  // -- shadowmap drawing pass
  GPU_RenderpassDesc rpass_desc = { .default_framebuffer = false,
                                    .has_color_target = false,
                                    .has_depth_stencil = true,
                                    .depth_stencil = depthmap };

  storage->shadowmap_pass = GPU_Renderpass_Create(rpass_desc);

  WARN("About to laod shaders");
  WARN("Shader paths: %s %s", "assets/shaders/shadows.vert", "assets/shaders/shadows.frag");
  Str8 vert_path = str8("assets/shaders/shadows.vert");
  Str8 frag_path = str8("assets/shaders/shadows.frag");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk");
  }

  ShaderDataLayout uniforms = ShadowUniforms_GetLayout(NULL);

  GraphicsPipelineDesc pipeline_desc = {
    .debug_name = "Shadows Pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = { uniforms },
    .data_layouts_count = 1,
    .vs = { .debug_name = "Shadows Vert shader",
            .filepath = vert_path,
            .code = vertex_shader.contents,
            .is_spirv = false },
    .fs = { .debug_name = "Shadows Frag shader",
            .filepath = frag_path,
            .code = fragment_shader.contents,
            .is_spirv = false },
  };
  storage->shadowmap_pipeline = GPU_GraphicsPipeline_Create(pipeline_desc, storage->shadowmap_pass);

  // -- debug quad pipeline
  GPU_RenderpassDesc debug_pass_desc = { .default_framebuffer = true };
  storage->debugquad_pass = GPU_Renderpass_Create(debug_pass_desc);

  vert_path = str8("assets/shaders/debug_quad.vert");
  frag_path = str8("assets/shaders/debug_quad.frag");
  vertex_shader = str8_from_file(&scratch, vert_path);
  fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk");
  }

  ShaderDataLayout debugquad_uniforms = ShadowDebugQuad_GetLayout(NULL);

  GraphicsPipelineDesc debugquad_pipeline_desc = {
    .debug_name = "Shadows debug quad Pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = { debugquad_uniforms },
    .data_layouts_count = 1,
    .vs = { .debug_name = "depth debug quad vert shader",
            .filepath = vert_path,
            .code = vertex_shader.contents,
            .is_spirv = false },
    .fs = { .debug_name = "depth debug quad frag shader",
            .filepath = frag_path,
            .code = fragment_shader.contents,
            .is_spirv = false },
  };
  storage->debugquad_pipeline =
      GPU_GraphicsPipeline_Create(debugquad_pipeline_desc, storage->debugquad_pass);

  Geometry quad_geo = Geo_CreatePlane(f32x2(1, 1));
  // HACK: Swap vertices to make it face us
  Vertex top0 = quad_geo.vertices->data[0];
  quad_geo.vertices->data[0] = quad_geo.vertices->data[2];
  quad_geo.vertices->data[2] = top0;
  Vertex top1 = quad_geo.vertices->data[1];
  quad_geo.vertices->data[1] = quad_geo.vertices->data[3];
  quad_geo.vertices->data[3] = top1;
  storage->quad = Mesh_Create(&quad_geo, false);

  arena_free_storage(&scratch);
}

void Shadow_Run(RenderEnt* entities, size_t entity_count) {
  Shadow_Storage* shadow_storage = Render_GetShadowStorage();

  // calculations
  RenderScene* render_scene = Render_GetScene();
  f32 near_plane = 1.0, far_plane = 10.0;
  // -- Not sure about how we want to handle lights
  Vec3 light_position = { 1, 4, -1 };
  // --
  Mat4 light_projection = mat4_orthographic(-10.0, 10.0, -10.0, 10.0, near_plane, far_plane);
  Mat4 light_view = mat4_look_at(light_position, VEC3_ZERO, VEC3_Y);
  Mat4 light_space_matrix = mat4_mult(light_view, light_projection);

  Shadow_ShadowmapExecute(shadow_storage, light_space_matrix, entities, entity_count);
}

void Shadow_DrawDebugQuad() {
  Shadow_Storage* shadow_storage = Render_GetShadowStorage();

  GPU_CmdEncoder* enc = GPU_GetDefaultEncoder();
  GPU_CmdEncoder_BeginRender(enc, shadow_storage->debugquad_pass);

  GPU_EncodeBindPipeline(enc, shadow_storage->debugquad_pipeline);
  ShaderDataLayout quad_data = ShadowDebugQuad_GetLayout(&shadow_storage->depth_texture);
  GPU_EncodeBindShaderData(enc, 0, quad_data);
  GPU_EncodeSetVertexBuffer(enc, shadow_storage->quad.vertex_buffer);
  GPU_EncodeSetIndexBuffer(enc, shadow_storage->quad.index_buffer);
  GPU_EncodeDrawIndexed(enc, shadow_storage->quad.geometry.indices->len);

  GPU_CmdEncoder_EndRender(enc);
}

void Shadow_ShadowmapExecute(Shadow_Storage* storage, Mat4 light_space_transform,
                             RenderEnt* entities, size_t entity_count) {
  GPU_CmdEncoder shadow_encoder = GPU_CmdEncoder_Create();

  GPU_CmdEncoder_BeginRender(&shadow_encoder, storage->shadowmap_pass);
  // DEBUG("Begin shadowmap renderpass");

  // FIXME: shouldnt be gl specific
  glClear(GL_DEPTH_BUFFER_BIT);

  GPU_EncodeBindPipeline(&shadow_encoder, storage->shadowmap_pipeline);

  ShadowUniforms uniforms = {
    .light_space = light_space_transform,
    .model = mat4_ident()  // this will be overwritten for each Model
  };
  ShaderDataLayout shader_data = ShadowUniforms_GetLayout(&uniforms);

  for (size_t ent_i = 0; ent_i < entity_count; ent_i++) {
    RenderEnt renderable = entities[ent_i];
    if (renderable.flags && REND_ENT_CASTS_SHADOWS) {
      // Model* model = MODEL_GET(renderable.model);

      uniforms.model = renderable.affine;  // update the model transform

      Mesh* mesh = Mesh_pool_get(Render_GetMeshPool(), renderable.mesh);
      GPU_EncodeBindShaderData(&shadow_encoder, 0, shader_data);
      GPU_EncodeSetVertexBuffer(&shadow_encoder, mesh->vertex_buffer);
      GPU_EncodeSetIndexBuffer(&shadow_encoder, mesh->index_buffer);
      GPU_EncodeDrawIndexed(&shadow_encoder, mesh->geometry.indices->len);
    }
  }

  GPU_CmdEncoder_EndRender(&shadow_encoder);  // end renderpass
}

TextureHandle Shadow_GetShadowMapTexture(Shadow_Storage* storage) { return storage->depth_texture; }
