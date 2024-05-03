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
#include "maths_types.h"
#include "ral.h"
#include "render.h"

// Shadowmap pass
// Blinn-phong pass
// Unlit pass
// Debug visualisations pass

typedef struct render_entity {
  model* model;
  transform tf;
} render_entity;

// Don't need to pass in *anything*.
gpu_renderpass* renderpass_blinn_phong_create();
void renderpass_blinn_phong_execute(gpu_renderpass* pass, render_entity* entities,
                                    size_t entity_count);

gpu_renderpass* renderpass_shadows_create();
void renderpass_shadows_execute(gpu_renderpass* pass, render_entity* entities, size_t entity_count);