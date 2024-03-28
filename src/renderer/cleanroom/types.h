#pragma once
#include "darray.h"
#include "defines.h"
#include "str.h"

typedef int texture_handle;
typedef int buffer_handle;
typedef int model_handle;

/** @brief Texture Description - used by texture creation functions */
typedef struct texture_desc {
  // gpu_texture_type tex_type;
  // gpu_texture_format format;
  // u32x2 extents;
} texture_desc;

/* 
 - render_types.h
 - ral_types.h 
 - ral.h
 - render.h ?
*/

// gpu types
typedef enum gpu_primitive_topology {
  CEL_PRIMITIVE_TOPOLOGY_POINT,
  CEL_PRIMITIVE_TOPOLOGY_LINE,
  CEL_PRIMITIVE_TOPOLOGY_LINE_STRIP,
  CEL_PRIMITIVE_TOPOLOGY_TRIANGLE,
  CEL_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
  CEL_PRIMITIVE_TOPOLOGY_COUNT
} cel_primitive_topology;

typedef enum gpu_texture_type {
  CEL_TEXTURE_TYPE_2D,
  CEL_TEXTURE_TYPE_3D,
  CEL_TEXTURE_TYPE_2D_ARRAY,
  CEL_TEXTURE_TYPE_CUBE_MAP,
  CEL_TEXTURE_TYPE_COUNT
} gpu_texture_type;

typedef enum gpu_texture_format {
  CEL_TEXTURE_FORMAT_8_8_8_8_RGBA_UNORM,
  CEL_TEXTURE_FORMAT_DEPTH_DEFAULT,
  CEL_TEXTURE_FORMAT_COUNT
} gpu_texture_format;

/* render_types */
typedef struct mesh mesh;
typedef struct model model;
typedef struct model pbr_material;
typedef struct model bp_material; // blinn-phong

#include "maths_types.h"

typedef enum vertex_format {
  VERTEX_STATIC_3D,
  VERTEX_SPRITE,
  VERTEX_COUNT
} vertex_format;

typedef union vertex {
  struct {
    vec3 position;
    vec4 colour;
    vec2 tex_coords;
    vec3 normal;
  } static_3d; /** @brief standard vertex format for static geometry in 3D */

  struct {
    vec2 position;
    vec4 colour;
    vec2 tex_coords;
  } sprite;

  // TODO: animated 3d
} vertex;

KITC_DECL_TYPED_ARRAY(vertex)

typedef struct geometry_data {
  vertex_format format;
  vertex_darray vertices;
} geometry_data;

typedef struct mesh {
  buffer_handle vertex_buffer;
  buffer_handle index_buffer;
  u32 index_count;
  bool has_indices;
  geometry_data* vertices; // NULL means it has been freed
} mesh;

typedef struct model {
  str8 debug_name;
  mesh* meshes;
  u32 mesh_count;
} model;

// Three registers
// 1. low level graphics api calls "ral"
// 2. higher level render calls
// 3. simplified immediate mode API

/* render.h */
// frontend -- these can be called from say a loop in an example, or via FFI
texture_handle texture_create(const char* debug_name, texture_desc description, const u8* data);

void texture_data_upload(texture_handle texture);
buffer_handle buffer_create(const char* debug_name, u64 size);

// models and meshes are implemented **in terms of the above**
mesh mesh_create();
model_handle model_load(const char* filepath);

/* ral.h */
// backend -- these are not seen by the higher-level code
typedef struct gpu_swapchain gpu_swapchain;
typedef struct gpu_device gpu_device;
typedef struct gpu_pipeline gpu_pipeline;

void gpu_texture_init();
void gpu_texture_upload();
void gpu_buffer_init();
void gpu_buffer_upload();

// command buffer gubbins

// 3. SIMA (simplified immediate mode api)
//      - dont need to worry about uploading mesh data
//      - very useful for debugging
void imm_draw_cuboid();
void imm_draw_sphere(vec3 pos, f32 radius, vec4 colour);
void imm_draw_camera_frustum();
static void imm_draw_model(const char* model_filepath); // tracks internally whether the model is loaded

static void imm_draw_model(const char* model_filepath) {
  // check that model is loaded
  // if not loaded, load model and upload to gpu - LRU cache for models
  // else submit draw call
}