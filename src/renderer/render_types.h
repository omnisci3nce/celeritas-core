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

/** @brief configuration passed to the renderer at init time */
typedef struct renderer_config {
  char window_name[256];
  u32 scr_width, scr_height;
  vec3 clear_colour; /** colour that the screen gets cleared to every frame */
} renderer_config;

typedef struct renderer {
  struct GLFWwindow* window;
  void* backend_context;
  renderer_config config;
  gpu_device device;
  gpu_swapchain swapchain;
  gpu_renderpass default_renderpass;
  gpu_pipeline static_opaque_pipeline;
  bool frame_aborted;
  struct resource_pools* resource_pools;
} renderer;

typedef struct geometry_data {
  vertex_format format;
  vertex_darray* vertices;  // TODO: make it not a pointer
  bool has_indices;
  u32_darray indices;
  vec3 colour; /** Optional: set vertex colours */
} geometry_data;

// 'Upload' a geometry_data (to GPU) -> get back a mesh
typedef struct mesh {
  buffer_handle vertex_buffer;
  buffer_handle index_buffer;
  u32 index_count;
  bool has_indices;
  geometry_data* vertices;  // NULL means it has been freed
  bool is_uploaded;
  bool is_latent;
} mesh;

/* Hot reloading:
C side - reload_model():
  - load model from disk using existing loader
  - remove from transform graph so it isnt tried to be drawn
*/

typedef struct model {
  str8 name;
  mesh* meshes;
  u32 mesh_count;
} model;

typedef struct texture {
} texture;

typedef struct texture_data {
  texture_desc description;
  void* image_data;
} texture_data;

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
typedef blinn_phong_material material;

// the default blinn-phong material. MUST be initialised with the function below
extern material DEFAULT_MATERIAL;
void default_material_init();

#ifndef TYPED_MESH_ARRAY
KITC_DECL_TYPED_ARRAY(mesh)
#define TYPED_MESH_ARRAY
#endif

#ifndef TYPED_MODEL_ARRAY
KITC_DECL_TYPED_ARRAY(model)
#define TYPED_MODEL_ARRAY
#endif

#ifndef TYPED_MATERIAL_ARRAY
KITC_DECL_TYPED_ARRAY(material)
#define TYPED_MATERIAL_ARRAY
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
