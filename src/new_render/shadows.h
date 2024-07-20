/**
 * @brief Functions for adding shadows to scene rendering.
 */

#pragma once
#include "defines.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render_types.h"

typedef struct Shadow_Storage {
  GPU_Renderpass* shadowmap_pass;
  GPU_Pipeline* shadowmap_pipeline;
  TextureHandle depth_texture;
  bool debug_quad_enabled;
  Mesh quad;
  GPU_Renderpass* debugquad_pass;
  GPU_Pipeline* debugquad_pipeline;
  // TODO: Some statistics tracking
} Shadow_Storage;

typedef struct ShadowUniforms {
  Mat4 light_space;
  Mat4 model;
} ShadowUniforms;

typedef struct Camera Camera;
typedef struct Mat4 Mat4;

// --- Public API
PUB void Shadow_Init(Shadow_Storage* storage, u32 shadowmap_width, u32 shadowmap_height);

/** @brief Run shadow map generation for given entities, and store in a texture.
 *  @note Uses active directional light for now */
PUB void Shadow_Run(RenderEnt* entities, size_t entity_count);

PUB void Shadow_DrawDebugQuad();

/** @brief Get the shadow texture generated from shadowmap pass */
PUB Handle Shadow_GetShadowMapTexture(Shadow_Storage* storage);

// --- Internal
GPU_Renderpass* Shadow_RPassCreate();                        // Creates the render pass
GPU_Pipeline* Shadow_PipelineCreate(GPU_Renderpass* rpass);  // Creates the pipeline
void Shadow_ShadowmapExecute(Shadow_Storage* storage, Mat4 light_space_transform,
                             RenderEnt* entities, size_t entity_count);
void Shadow_RenderDebugQuad();
