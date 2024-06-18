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

#include <assert.h>
#include "defines.h"
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
  f32 pad[2];
} pbr_params_material_uniforms;

typedef struct pbr_point_light {
  vec3 pos;
  f32 pad;
  vec3 color;
  f32 pad2;
} pbr_point_light;

typedef struct pbr_params_light_uniforms {
  pbr_point_light pointLights[4];
  vec4 viewPos;
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

  if (has_data) {
  // printf("Size %d \n", b3.data.bytes.size);
    b1.data.bytes.data = &d->mvp_matrices;
    b2.data.bytes.data = &d->material;
    /* d->lights.viewPos = vec3(0, 1, 0); */
    b3.data.bytes.data = &d->lights;
    // print_vec3(d->lights.viewPos);
  }

  return (shader_data_layout){ .name = "pbr_params", .bindings = { b1, b2, b3 }, .bindings_count = 3

  };
}

static void* shader_layout_get_binding(shader_data_layout* layout, u32 nth_binding) {
  assert(nth_binding < layout->bindings_count);
  return &layout->bindings[nth_binding].data;
}

typedef struct pbr_textures {
  texture_handle albedo_tex;
  texture_handle metal_roughness_tex;
  texture_handle ao_tex;
  texture_handle normal_tex;
} pbr_textures;

typedef struct pbr_textured_bindgroup {
  mvp_matrix_uniforms mvp_matrices;
  pbr_params_light_uniforms lights;
  pbr_textures textures;
} pbr_textured_bindgroup;

static shader_data_layout pbr_textured_shader_layout(void* data) {
  pbr_textured_bindgroup* d = (pbr_textured_bindgroup*)data;
  bool has_data = data != NULL;

  shader_binding b1 = { .label = "MVP_Matrices",
                        .type = SHADER_BINDING_BYTES,
                        .stores_data = has_data,
                        .data = { .bytes = { .size = sizeof(mvp_matrix_uniforms) } } };

  shader_binding b2 = { .label = "Scene_Lights",
                        .type = SHADER_BINDING_BYTES,
                        .stores_data = has_data,
                        .data = { .bytes = { .size = sizeof(pbr_params_light_uniforms) } } };

  shader_binding b3 = {.label = "Albedo",
                        .type = SHADER_BINDING_TEXTURE,
                        .stores_data = has_data };
  shader_binding b4 = {.label = "Metallic Roughness",
                        .type = SHADER_BINDING_TEXTURE,
                        .stores_data = has_data };
  shader_binding b5 = {.label = "Ambient Occlusion",
                        .type = SHADER_BINDING_TEXTURE,
                        .stores_data = has_data };
 shader_binding b6 = {.label = "Normal Vectors",
                        .type = SHADER_BINDING_TEXTURE,
                        .stores_data = has_data };

  if (has_data) {
    b1.data.bytes.data = &d->mvp_matrices;
    b2.data.bytes.data = &d->lights;
    b3.data.texture.handle = d->textures.albedo_tex;
    b4.data.texture.handle = d->textures.metal_roughness_tex;
    b5.data.texture.handle = d->textures.ao_tex;
    b6.data.texture.handle = d->textures.normal_tex;
  }

  return (shader_data_layout){ .name = "pbr_params", .bindings = { b1, b2, b3, b4, b5, b6  }, .bindings_count = 6
  };
}
