// /**
//  * @file render_types.h
//  * @author Omniscient
//  * @brief Type definitions for the majority of data required by the renderer system
//  * @date 2024-02-24
//  *
//  */
// #pragma once

#include "animation.h"
#include "darray.h"
#include "maths.h"
#include "maths_types.h"
#include "str.h"

// struct GLFWwindow;

// #define MAX_MATERIAL_NAME_LEN 256
// #define MAX_TEXTURE_NAME_LEN 256

// #ifndef RESOURCE_HANDLE_DEFS
// // CORE_DEFINE_HANDLE(model_handle);
// #define ABSENT_MODEL_HANDLE 999999999
// // CORE_DEFINE_HANDLE(texture_handle);
// #define RESOURCE_HANDLE_DEFS
// #endif

// /* @brief Opaque wrapper around a shader program */
// typedef struct shader {
//   u32 program_id;
// } shader;

// /** @brief configuration passed to the renderer at init time */
// typedef struct renderer_config {
//   char window_name[256];
//   u32 scr_width, scr_height;
//   vec3 clear_colour; /** colour that the screen gets cleared to every frame */
// } renderer_config;

// typedef struct frame_stats {
//   u64 last_time;
// } frame_stats;

typedef struct renderer {
  struct GLFWwindow *window; /** Currently all platforms use GLFW*/
  void *backend_state;       /** Graphics API-specific state */
  renderer_config config;
  // shaders
  shader blinn_phong;
  shader skinned;
} renderer;

// // --- Lighting & Materials

typedef struct texture {
  u32 texture_id;
  char name[MAX_TEXTURE_NAME_LEN];
  void *image_data;
  void *backend_data;
  u32 width;
  u32 height;
  u8 channel_count;
  u32 channel_type;
} texture;

// typedef struct blinn_phong_material {
//   char name[MAX_MATERIAL_NAME_LEN];
//   texture diffuse_texture;
//   char diffuse_tex_path[256];
//   texture specular_texture;
//   char specular_tex_path[256];
//   vec3 ambient_colour;
//   vec3 diffuse;
//   vec3 specular;
//   f32 spec_exponent;
//   bool is_loaded;
//   bool is_uploaded;
// } blinn_phong_material;
// typedef blinn_phong_material material;  // when we start using PBR, this will no longer be the case

// // the default blinn-phong material. MUST be initialised with the function below
// extern material DEFAULT_MATERIAL;
// void default_material_init();

#ifndef TYPED_MATERIAL_ARRAY
KITC_DECL_TYPED_ARRAY(material)  // creates "material_darray"
#define TYPED_MATERIAL_ARRAY
#endif

#ifndef TYPED_ANIMATION_CLIP_ARRAY
KITC_DECL_TYPED_ARRAY(animation_clip)  // creates "material_darray"
#define TYPED_ANIMATION_CLIP_ARRAY
#endif

// // lights
// typedef struct point_light {
//   vec3 position;
//   f32 constant, linear, quadratic;
//   vec3 ambient;
//   vec3 diffuse;
//   vec3 specular;
// } point_light;

// typedef struct directional_light {
//   vec3 direction;
//   vec3 ambient;
//   vec3 diffuse;
//   vec3 specular;
// } directional_light;

// void point_light_upload_uniforms(shader shader, point_light *light, char index);
// void dir_light_upload_uniforms(shader shader, directional_light *light);

// // --- Models & Meshes

// /** @brief Vertex format for a static mesh */
// typedef struct vertex {
//   vec3 position;
//   vec3 normal;
//   vec2 uv;
// } vertex;

typedef struct vertex_bone_data {
  vec4u joints; /** @brief 4 indices of joints that influence vectors position */
  vec4 weights; /** @brief weight (0,1) of each joint */
} vertex_bone_data;

#include "animation.h"
#ifndef TYPED_VERTEX_ARRAY
KITC_DECL_TYPED_ARRAY(vertex)            // creates "vertex_darray"
KITC_DECL_TYPED_ARRAY(vertex_bone_data)  // creates "skinned_vertex_darray"
KITC_DECL_TYPED_ARRAY(joint)
#define TYPED_VERTEX_ARRAY
#endif

typedef struct mesh {
  vertex_darray *vertices;
  vertex_bone_data_darray *vertex_bone_data;  // only used if model needs it
  joint_darray *bones;
  bool is_skinned;
  u32 vertex_size; /** size in bytes of each vertex including necessary padding */
  bool has_indices;
  u32 *indices;
  u32 indices_len;
  size_t material_index;
  u32 vbo, vao; /** OpenGL data. TODO: dont leak OpenGL details */
} mesh;

// #ifndef TYPED_MESH_ARRAY
// KITC_DECL_TYPED_ARRAY(mesh)  // creates "mesh_darray"
// #define TYPED_MESH_ARRAY
// #endif

typedef struct model {
  str8 name;
  mesh_darray *meshes;
  aabb_3d bbox;
  material_darray *materials;
  animation_clip_darray *animations;
  arena animation_data_arena;
  bool is_loaded;
  bool is_uploaded;
} model;

// #ifndef TYPED_MODEL_ARRAY
// KITC_DECL_TYPED_ARRAY(model)  // creates "model_darray"
// #define TYPED_MODEL_ARRAY
// #endif

// // --- Scene

// // NOTE: This struct won't stay like this for a long time. It's somewhat temporary
// //       in order to get a basic scene working without putting burden on the caller of
// //       draw_model()
// typedef struct scene {
//   directional_light dir_light;
//   point_light point_lights[4];
//   size_t n_point_lights;
// } scene;

// // --- Graphics API related

// // typedef enum cel_primitive_topology {
// //   CEL_PRIMITIVE_TOPOLOGY_POINT,
// //   CEL_PRIMITIVE_TOPOLOGY_LINE,
// //   CEL_PRIMITIVE_TOPOLOGY_LINE_STRIP,
// //   CEL_PRIMITIVE_TOPOLOGY_TRIANGLE,
// //   CEL_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
// //   CEL_PRIMITIVE_TOPOLOGY_COUNT
// // } cel_primitive_topology;

// // typedef enum gpu_texture_type {
// //   TEXTURE_TYPE_2D,
// //   TEXTURE_TYPE_3D,
// //   TEXTURE_TYPE_2D_ARRAY,
// //   TEXTURE_TYPE_CUBE_MAP,
// //   TEXTURE_TYPE_COUNT
// // } gpu_texture_type;

// // typedef enum gpu_texture_format {
// //   TEXTURE_FORMAT_8_8_8_8_RGBA_UNORM,
// //   TEXTURE_FORMAT_DEPTH_DEFAULT,
// //   TEXTURE_FORMAT_COUNT
// // } gpu_texture_format;

// // typedef enum pipeline_kind {
// //   GRAPHICS,
// //   COMPUTE,
// // } pipeline_kind;