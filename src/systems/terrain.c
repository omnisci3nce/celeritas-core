/**
 * @file terrain.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-06-22
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "terrain.h"
#include "ral.h"

bool terrain_system_init(terrain_state* state) {
  gpu_renderpass_desc rpass_desc = {
    .default_framebuffer = true,
  };
  struct graphics_pipeline_desc pipeline_desc = {

  };

  state->hmap_renderpass = gpu_renderpass_create(&rpass_desc);
  state->hmap_pipeline = gpu_graphics_pipeline_create(pipeline_desc);
}