/**
 * @file builtin_materials.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-06-15
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "colours.h"
#include "defines.h"
#include "ral.h"
#include "ral_types.h"

// Currently supported materials
// - Blinn Phong (textured)
// - PBR (params)
// - PBR (textured)

// Thoughts
// --------
//
// A material and a shader are inextricably linked. The input data for a shader needs the material.
// However, a shader may require more than just a material?

// --- Common uniform blocks

/* In glsl code we call it 'MVP_Matrices' */
typedef struct mvp_matrix_uniforms {
  mat4 model;
  mat4 view;
  mat4 projection;
} mvp_matrix_uniforms;

// --- PBR (params)

typedef struct pbr_params_material_uniforms {
  vec3 albedo;
  f32 metallic;
  f32 roughness;
  f32 ao;
} pbr_params_material_uniforms;

typedef struct pbr_point_light {
  vec3 pos;
  vec3 color;
} pbr_point_light;

typedef struct pbr_params_light_uniforms {
  vec3 viewPos;
  pbr_point_light pointLights[4];
} pbr_params_light_uniforms;

typedef struct pbr_params_bindgroup {
  mvp_matrix_uniforms mvp_matrices;
  pbr_params_material_uniforms material;
  pbr_params_light_uniforms lights;
} pbr_params_bindgroup;

static shader_data_layout pbr_params_shader_layout(void* data) {
  pbr_params_bindgroup* d = (pbr_params_bindgroup*)data;
  bool has_data = data != NULL;

  shader_binding b1 = { .label = "MVP_Matrices",
                        .type = SHADER_BINDING_BYTES,
                        .stores_data = has_data,
                        .data = { .bytes = { .size = sizeof(mvp_matrix_uniforms) } } };

  shader_binding b2 = { .label = "PBR_Params",
                        .type = SHADER_BINDING_BYTES,
                        .stores_data = has_data,
                        .data = { .bytes = { .size = sizeof(pbr_params_material_uniforms) } } };

  shader_binding b3 = { .label = "Scene_Lights",
                        .type = SHADER_BINDING_BYTES,
                        .stores_data = has_data,
                        .data = { .bytes = { .size = sizeof(pbr_params_light_uniforms) } } };

  return (shader_data_layout){ .name = "pbr_params", .bindings = { b1, b2, b3 }, .bindings_count = 3

  };
}
