/**
 * @brief Functions for adding shadows to scene rendering.
*/


#pragma once
#include "defines.h"
#include "ral.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render_types.h"

typedef struct Shadow_Storage {
    GPU_Renderpass* shadowmap_pass;
    GPU_Pipeline* pipeline;
    bool debug_quad_enabled;
    TextureHandle depth_texture;
    // TODO: Some statistics tracking
} Shadow_Storage;

typedef struct Camera Camera;
typedef struct Mat4 Mat4;

// --- Public API
PUB void Shadow_Init(Shadow_Storage* storage, u32x2 shadowmap_extents);

/** @brief Run shadow map generation for given entities, and store in a texture.
 *  @note Uses active directional light for now */
PUB void Shadow_Run(Shadow_Storage* storage, RenderEnt* entities, size_t entity_count);

/** @brief Get the shadow texture generated from shadowmap pass */
PUB Handle Shadow_GetShadowMapTexture(Shadow_Storage* storage);

// --- Internal
GPU_Renderpass* Shadow_RPassCreate(); // Creates the render pass
GPU_Pipeline*   Shadow_PipelineCreate(GPU_Renderpass* rpass); // Creates the pipeline
void Shadow_ShadowmapExecute(Shadow_Storage* storage, Mat4 light_space_transform, RenderEnt* entities, size_t entity_count);
void Shadow_RenderDebugQuad();
