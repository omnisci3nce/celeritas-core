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
#include "maths_types.h"
#include "ral.h"
#include "ral_types.h"

#define SHADOW_WIDTH 1000
#define SHADOW_HEIGHT 1000


gpu_renderpass* shadowmaps_renderpass_create() {
  // Create depthmap texture
  u32x2 extents = u32x2(SHADOW_WIDTH, SHADOW_HEIGHT);
  texture_desc depthmap_desc = {
    .extents = extents,
    .format = CEL_TEXTURE_FORMAT_DEPTH_DEFAULT,
    .tex_type = CEL_TEXTURE_TYPE_2D
  };
  texture_handle depthmap = gpu_texture_create(depthmap_desc, false, NULL);

  gpu_renderpass_desc shadows_desc = {
    .default_framebuffer = false,
    .has_color_target = false,
    .has_depth_stencil = true,
    .depth_stencil = depthmap
  };
  return gpu_renderpass_create(&shadows_desc);
}

gpu_pipeline* shadowmaps_pipeline_create() {
  struct graphics_pipeline_desc desc = {
    .
  };
  gpu_graphics_pipeline_create(struct graphics_pipeline_desc description)
}

void renderpass_shadowmap_execute(gpu_renderpass* pass, render_entity* entities, size_t entity_count) {

}