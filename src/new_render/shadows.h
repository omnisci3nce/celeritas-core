/**
 * @file shadows.h
 * @brief Functions for adding shadows to scene rendering.
*/


#pragma once
#include "defines.h"
#include "ral/ral_types.h"

typedef struct Shadow_Storage Shadow_Storage;

typedef struct RenderEnt RenderEnt;
typedef struct Camera Camera;
typedef struct Mat4 Mat4;

// --- Public API
PUB void Shadow_Init(Shadow_Storage* storage);

/** @brief Run shadow map generation for given entities, and store in a texture.
 *  @note Uses active directional light for now */
PUB void Shadow_Run(Shadow_Storage* storage, RenderEnt* entities, size_t entity_count);

/** @brief Get the shadow texture generated from shadowmap pass */
PUB Handle Shadow_GetShadowMapTexture(Shadow_Storage* storage);

// --- Internal
GPU_Renderpass* Shadow_RPassCreate(); // Creates the render pass
GPU_Pipeline*   Shadow_PipelineCreate(GPU_Renderpass* rpass); // Creates the pipeline
void Shadow_ShadowmapExecute(Shadow_Storage* storage, Mat4 light_space_transform, RenderEnt* entites, size_t entity_count);
