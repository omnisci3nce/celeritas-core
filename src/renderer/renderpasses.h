/**
 * @file renderpasses.h
 * @author your name (you@domain.com)
 * @brief Built-in renderpasses to the engine
 * @version 0.1
 * @date 2024-04-28
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include "ral.h"
#include "render_types.h"

// Shadowmap pass
// Blinn-phong pass
// Unlit pass
// Debug visualisations pass

// Don't need to pass in *anything*.
gpu_renderpass* renderpass_blinn_phong_create();
void renderpass_blinn_phong_execute(gpu_renderpass* pass, render_entity* entities,
                                    size_t entity_count);

typedef struct ren_shadowmaps {
  u32 width;
  u32 height;
  gpu_renderpass* rpass;
  gpu_pipeline* static_pipeline;
} ren_shadowmaps;

void ren_shadowmaps_init(ren_shadowmaps* storage);

gpu_renderpass* shadowmaps_renderpass_create();
gpu_pipeline* shadowmaps_pipeline_create(gpu_renderpass* rpass);

void renderpass_shadowmap_execute(gpu_renderpass* pass, render_entity* entities, size_t entity_count);
