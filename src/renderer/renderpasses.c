/**
 * @file renderpasses.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-06-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "renderpasses.h"
#include "file.h"
#include "log.h"
#include "maths_types.h"
#include "ral.h"
#include "ral_types.h"

#define SHADOW_WIDTH 1000
#define SHADOW_HEIGHT 1000

shader_data_layout debug_quad_layout(void* data) {
  debug_quad_uniform* d = data;
  bool has_data = data != NULL;

  shader_binding b1 = { .label = "depthMap",
                        .type = SHADER_BINDING_TEXTURE,
                        .stores_data = has_data};
  if (has_data) {
    b1.data.texture.handle = d->depthMap;
  }
  return (shader_data_layout){ .name = "debug quad uniforms", .bindings = { b1 }, .bindings_count = 1 };
}

gpu_pipeline* debug_quad_pipeline_create() {
  gpu_renderpass_desc rpass_desc = {.default_framebuffer = true };
  gpu_renderpass* rpass = gpu_renderpass_create(&rpass_desc);
  shader_data shader_layout = {.data = NULL, .shader_data_get_layout = debug_quad_layout};
  struct graphics_pipeline_desc desc = {
    .debug_name = "Shadow maps debug quad",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = { shader_layout },
    .data_layouts_count = 1,
    .vs = shader_quick_load("assets/shaders/debug_quad.vert"),
    .fs = shader_quick_load("assets/shaders/debug_quad.frag"),
    .renderpass = rpass,
  };

  return gpu_graphics_pipeline_create(desc);
}

void ren_shadowmaps_init(ren_shadowmaps* storage) {
  storage->rpass = shadowmaps_renderpass_create();
  storage->static_pipeline = shadowmaps_pipeline_create(storage->rpass);
  storage->debug_quad = debug_quad_pipeline_create();
  storage->depth_tex = storage->rpass->description.depth_stencil;
}

gpu_renderpass* shadowmaps_renderpass_create() {
  // Create depthmap texture
  u32x2 extents = u32x2(SHADOW_WIDTH, SHADOW_HEIGHT);
  texture_desc depthmap_desc = { .extents = extents,
                                 .format = CEL_TEXTURE_FORMAT_DEPTH_DEFAULT,
                                 .tex_type = CEL_TEXTURE_TYPE_2D };
  texture_handle depthmap = gpu_texture_create(depthmap_desc, false, NULL);

  gpu_renderpass_desc shadows_desc = { .default_framebuffer = false,
                                       .has_color_target = false,
                                       .has_depth_stencil = true,
                                       .depth_stencil = depthmap };
  return gpu_renderpass_create(&shadows_desc);
}

// == shader bindings

shader_data_layout model_uniform_layout(void* data) {
  bool has_data = data != NULL;

  shader_binding b1 = { .label = "Model",
                        .type = SHADER_BINDING_BYTES,
                        .stores_data = has_data,
                        .data = { .bytes.size = sizeof(model_uniform) } };
  if (has_data) {
    b1.data.bytes.data = data;
  }
  return (shader_data_layout){ .name = "model_uniform", .bindings = { b1 }, .bindings_count = 1 };
}
shader_data_layout lightspace_uniform_layout(void* data) {
  bool has_data = data != NULL;

  shader_binding b1 = { .label = "LightSpace",
                        .type = SHADER_BINDING_BYTES,
                        .stores_data = has_data,
                        .data = { .bytes.size = sizeof(lightspace_tf_uniform) } };
  if (has_data) {
    b1.data.bytes.data = data;
  }
  return (shader_data_layout){ .name = "lightspace_tf_uniform",
                               .bindings = { b1 },
                               .bindings_count = 1 };
}

// ==================

gpu_pipeline* shadowmaps_pipeline_create(gpu_renderpass* rpass) {
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  str8 vert_path = str8lit("assets/shaders/shadows.vert");
  str8 frag_path = str8lit("assets/shaders/shadows.frag");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk");
  }

  // We'll have two data layouts. 1. for the light-space transform, and 2. for the model matrix
  shader_data model_uniform = { .data = NULL, .shader_data_get_layout = &model_uniform_layout };
  shader_data lightspace_uniform = { .data = NULL,
                                     .shader_data_get_layout = &lightspace_uniform_layout };

  struct graphics_pipeline_desc desc = { .debug_name = "Shadowmap drawing pipeline",
                                         .vertex_desc = static_3d_vertex_description(),
                                         .data_layouts = { model_uniform, lightspace_uniform },
                                         .data_layouts_count = 2,
                                         .vs = { .debug_name = "Shadows Vert shader",
                                                 .filepath = vert_path,
                                                 .code = vertex_shader.contents,
                                                 .is_spirv = true },
                                         .fs = { .debug_name = "Shadows Frag shader",
                                                 .filepath = frag_path,
                                                 .code = fragment_shader.contents,
                                                 .is_spirv = true },
                                         .renderpass = rpass };

  arena_free_storage(&scratch);
  return gpu_graphics_pipeline_create(desc);
}

void renderpass_shadowmap_execute(gpu_renderpass* pass, render_entity* entities,
                                  size_t entity_count) {}