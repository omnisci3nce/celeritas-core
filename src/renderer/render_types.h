/**
 * @file render_types.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-27
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "colours.h"
#include "defines.h"
#include "ral.h"
#include "ral_types.h"
#if defined(CEL_PLATFORM_WINDOWS)
// #include "backend_dx11.h"
#endif
#if defined(CEL_REND_BACKEND_VULKAN)
#include "backend_vulkan.h"
#elif defined(CEL_REND_BACKEND_METAL)
#include "backend_metal.h"
#elif defined(CEL_REND_BACKEND_OPENGL)
#include "backend_opengl.h"
#endif

struct GLFWwindow;

typedef struct geometry_data {
  vertex_format format;
  vertex_darray* vertices;  // TODO: make it not a pointer
  bool has_indices;
  u32_darray* indices;
  rgba colour; /** Optional: set vertex colours */
} geometry_data;

typedef struct u32_opt {
  u32 value;
  bool has_value;
} u32_opt;

// 'Upload' a geometry_data (to GPU) -> get back a mesh
typedef struct mesh {
  buffer_handle vertex_buffer;
  buffer_handle index_buffer;
  geometry_data* geometry;  // NULL means it has been freed
  u32_opt material_index;
  bool is_uploaded;
  bool is_latent;
} mesh;

#ifndef TYPED_MESH_ARRAY
KITC_DECL_TYPED_ARRAY(mesh)
#define TYPED_MESH_ARRAY
#endif

/* Hot reloading:
C side - reload_model():
  - load model from disk using existing loader
  - remove from transform graph so it isnt tried to be drawn
*/

typedef struct texture {
} texture;

typedef struct texture_data {
  texture_desc description;
  void* image_data;
} texture_data;

typedef enum material_kind {
  MAT_BLINN_PHONG,
  MAT_PBR,
  MAT_PBR_PARAMS,  // uses float values to represent a surface uniformly
  MAT_COUNT
} material_kind;
static const char* material_kind_names[] = { "Blinn Phong", "PBR (Textures)", "PBR (Params)",
                                             "Count (This should be an error)" };

typedef struct blinn_phong_material {
  char name[256];
  texture diffuse_texture;
  char diffuse_tex_path[256];
  texture specular_texture;
  char specular_tex_path[256];
  vec3 ambient_colour;
  vec3 diffuse;
  vec3 specular;
  f32 spec_exponent;
  bool is_loaded;
  bool is_uploaded;
} blinn_phong_material;
// typedef blinn_phong_material material;

typedef struct pbr_parameters {
  vec3 albedo;
  f32 metallic;
  f32 roughness;
  f32 ao;
} pbr_parameters;

typedef struct pbr_material {
  texture_handle albedo_map;
  texture_handle normal_map;
  bool metal_roughness_combined;
  texture_handle metallic_map;
  texture_handle roughness_map;
  texture_handle ao_map;
} pbr_material;

typedef struct material {
  material_kind kind;
  union {
    blinn_phong_material blinn_phong;
    pbr_parameters pbr_params;
    pbr_material pbr;
  } mat_data;
  char* name;
} material;

#ifndef TYPED_MATERIAL_ARRAY
KITC_DECL_TYPED_ARRAY(material)
#define TYPED_MATERIAL_ARRAY
#endif

CORE_DEFINE_HANDLE(model_handle);

typedef struct model {
  str8 name;
  mesh_darray* meshes;
  material_darray* materials;
} model;

TYPED_POOL(model, model)

// FIXME: the default blinn-phong material. MUST be initialised with the function below
// FIXME: extern material DEFAULT_MATERIAL;
void default_material_init();

#ifndef TYPED_MODEL_ARRAY
KITC_DECL_TYPED_ARRAY(model)
#define TYPED_MODEL_ARRAY
#endif

#ifndef TYPED_ANIMATION_CLIP_ARRAY
#include "animation.h"
KITC_DECL_TYPED_ARRAY(animation_clip)
#define TYPED_ANIMATION_CLIP_ARRAY
#endif

/** @brief Describes all the data required for the renderer to start executing draws */
typedef struct render_entity {
  /* buffer_handle index_buffer; */
  /* u32 index_count; */
  /* u32 index_offset; */
  /* buffer_handle vertex_buffer; */
  model_handle model;
  transform tf;
} render_entity;

#ifndef TYPED_RENDER_ENTITY_ARRAY
KITC_DECL_TYPED_ARRAY(render_entity)
#define TYPED_RENDER_ENTITY_ARRAY
#endif

// --- Lights
typedef struct point_light {
  vec3 position;
  f32 constant, linear, quadratic;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
} point_light;

typedef struct directional_light {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
} directional_light;
