#include "shadows.h"
#include <string.h>
#include "core.h"
#include "file.h"
#include "log.h"
#include "maths.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render.h"
#include "render_scene.h"
#include "render_types.h"

typedef struct ShadowUniforms {
  Mat4 light_space;
  Mat4 model;
} ShadowUniforms;

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

void Shadow_Init(Shadow_Storage* storage, u32x2 shadowmap_extents) {
  memset(storage, 0, sizeof(Shadow_Storage));
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  TextureDesc depthmap_desc = { .extents = shadowmap_extents,
                                .format = TEXTURE_FORMAT_DEPTH_DEFAULT,
                                .tex_type = TEXTURE_TYPE_2D };
  TextureHandle depthmap = GPU_TextureCreate(depthmap_desc, false, NULL);
  storage->depth_texture = depthmap;

  GPU_RenderpassDesc rpass_desc = { .default_framebuffer = false,
                                    .has_color_target = false,
                                    .has_depth_stencil = true,
                                    .depth_stencil = depthmap };

  storage->shadowmap_pass = GPU_Renderpass_Create(rpass_desc);

  Str8 vert_path = str8("assets/shaders/shadows.vert");
  Str8 frag_path = str8("assets/shaders/shadows.frag");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk");
  }

  ShaderData uniforms = { .data = NULL, .get_layout = &ShadowUniforms_GetLayout };

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
  storage->pipeline = GPU_GraphicsPipeline_Create(pipeline_desc, storage->shadowmap_pass);

  arena_free_storage(&scratch);
}

void Shadow_ShadowmapExecute(Shadow_Storage* storage, Mat4 light_space_transform,
                             RenderEnt* entities, size_t entity_count) {
  GPU_CmdEncoder shadow_encoder = GPU_CmdEncoder_Create();

  GPU_CmdEncoder_BeginRender(&shadow_encoder, storage->shadowmap_pass);
  DEBUG("Begin shadowmap renderpass");

  GPU_EncodeBindPipeline(&shadow_encoder, storage->pipeline);

  ShadowUniforms uniforms = {
    .light_space = light_space_transform,
    .model = mat4_ident()  // this will be overwritten for each Model
  };
  ShaderData shader_data = {
    .data = &uniforms,
    .get_layout = &ShadowUniforms_GetLayout,
  };

  for (size_t ent_i = 0; ent_i < entity_count; ent_i++) {
    RenderEnt renderable = entities[ent_i];
    if (renderable.casts_shadows) {
      Model* model = MODEL_GET(renderable.model);

      uniforms.model = renderable.affine;  // update the model transform

      size_t num_meshes = Mesh_darray_len(model->meshes);
      for (u32 mesh_i = 0; mesh_i < num_meshes; mesh_i++) {
        Mesh mesh = model->meshes->data[mesh_i];

        GPU_EncodeBindShaderData(&shadow_encoder, 0, shader_data);
        GPU_EncodeSetVertexBuffer(&shadow_encoder, mesh.vertex_buffer);
        GPU_EncodeSetIndexBuffer(&shadow_encoder, mesh.index_buffer);
        GPU_EncodeDrawIndexed(&shadow_encoder, mesh.geometry->indices->len);
      }
    }
  }

  GPU_CmdEncoder_EndRender(&shadow_encoder);  // end renderpass
}

Handle Shadow_GetShadowMapTexture(Shadow_Storage* storage) {
  return (Handle){ .raw = storage->depth_texture.raw };
}
