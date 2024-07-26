/**
 * @file pbr.h
 * @brief PBR render pass
 */

#pragma once
#include "backend_opengl.h"
#include "camera.h"
#include "defines.h"
#include "maths_types.h"
#include "ral_types.h"
#include "render_types.h"

// --- Public API
typedef struct PBR_Storage {
  GPU_Renderpass* pbr_pass;
  GPU_Pipeline* pbr_pipeline;

} PBR_Storage;  // Stores all necessary data and handles

typedef struct PBRMaterialUniforms {
  Material mat;
} PBRMaterialUniforms;

PUB void PBR_Init(PBR_Storage* storage);

// NOTE: For simplicity's sake we will render this pass directly to the default framebuffer
PUB void PBR_Run(PBR_Storage* storage
                 // light data
                 // camera
                 // geometry
                 // materials
);

typedef struct PBR_Params {
  Vec3 albedo;
  f32 metallic;
  f32 roughness;
  f32 ambient_occlusion;
} PBR_Params;

typedef struct PBR_Textures {
  TextureHandle albedo_map;
  TextureHandle normal_map;
  bool metal_roughness_combined;
  TextureHandle metallic_map;
  TextureHandle roughness_map;
  TextureHandle ao_map;
} PBR_Textures;


// --- Internal

typedef struct MaterialMap MaterialMap;

Material PBRMaterialDefault();

GPU_Renderpass* PBR_RPassCreate();

GPU_Pipeline* PBR_PipelineCreate(GPU_Renderpass* rpass);

void PBR_Execute(PBR_Storage* storage, Camera camera, TextureHandle shadowmap_tex,
                 RenderEnt* entities, size_t entity_count);

ShaderDataLayout PBRMaterial_GetLayout(void* data);