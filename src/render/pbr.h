/**
 * @file pbr.h
 * @brief PBR render pass and uniforms
 */

#pragma once
#include "backend_opengl.h"
#include "camera.h"
#include "defines.h"
#include "maths_types.h"
#include "ral_types.h"
#include "render_types.h"

// --- Public API

/** @brief Holds data for the PBR pipeline */
typedef struct PBR_Storage {
  GPU_Renderpass* pbr_pass;
  GPU_Pipeline* pbr_pipeline;
} PBR_Storage;

typedef struct PBRMaterialUniforms {
  Material mat;
} PBRMaterialUniforms;

/** @brief  */
PUB void PBR_Init(PBR_Storage* storage);

// NOTE: For simplicity's sake we will render this pass directly to the default framebuffer
// internally this defers to `PBR_Execute()`
PUB void PBR_Run(PBR_Storage* storage
                 // light data
                 // camera
                 // geometry
                 // materials
);

/** @brief Parameters that get passed as a uniform block to the PBR shader */
typedef struct PBR_Params {
  Vec3 albedo;
  f32 metallic;
  f32 roughness;
  f32 ambient_occlusion;
} PBR_Params;

/** @brief Textures that will get passed into the PBR shader if they're not `INVALID_TEX_HANDLE` */
typedef struct PBR_Textures {
  TextureHandle albedo_map;
  TextureHandle normal_map;
  bool metal_roughness_combined;
  TextureHandle metallic_map;
  TextureHandle roughness_map;
  TextureHandle ao_map;
} PBR_Textures;

/** @brief Returns a default white matte material */
PUB Material PBRMaterialDefault();

PUB ShaderDataLayout PBRMaterial_GetLayout(void* data);

// --- Internal

GPU_Renderpass* PBR_RPassCreate(); /** @brief Create the PBR Renderpass */

GPU_Pipeline* PBR_PipelineCreate(GPU_Renderpass* rpass); /** @brief Create the PBR Pipeline */

void PBR_Execute(PBR_Storage* storage, Camera camera, TextureHandle shadowmap_tex,
                 RenderEnt* entities, size_t entity_count);
