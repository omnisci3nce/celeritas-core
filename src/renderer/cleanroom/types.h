#pragma once
#include "darray.h"
#include "defines.h"
#include "maths_types.h"
#include "str.h"

CORE_DEFINE_HANDLE(buffer_handle);
CORE_DEFINE_HANDLE(texture_handle);
CORE_DEFINE_HANDLE(sampler_handle);
CORE_DEFINE_HANDLE(shader_handle);
CORE_DEFINE_HANDLE(model_handle);

typedef struct transform_hierarchy {
} transform_hierarchy;

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
typedef struct model pbr_material;
typedef struct model bp_material;  // blinn-phong

#include "maths_types.h"

typedef enum vertex_format {
  VERTEX_STATIC_3D,
  VERTEX_SPRITE,
  VERTEX_SKINNED,
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
  } sprite; /** @brief vertex format for 2D sprites or quads */

  struct {
    vec3 position;
    vec4 colour;
    vec2 tex_coords;
    vec3 normal;
    vec4i bone_ids;     // Integer vector for bone IDs
    vec4 bone_weights;  // Weight of each bone's influence
  } skinned_3d;         /** @brief vertex format for skeletal (animated) geometry in 3D */
} vertex;

KITC_DECL_TYPED_ARRAY(vertex)
KITC_DECL_TYPED_ARRAY(u32)

typedef struct geometry_data {
  vertex_format format;
  vertex_darray vertices;
  bool has_indices;
  u32_darray indices;
  vec3 colour; /** Optional: set vertex colours */
} geometry_data;

void geo_set_vertex_colours(geometry_data* geo, vec4 colour);

typedef struct mesh {
  buffer_handle vertex_buffer;
  buffer_handle index_buffer;
  u32 index_count;
  bool has_indices;
  geometry_data* vertices;  // NULL means it has been freed
} mesh;

/* Hot reloading:
C side - reload_model():
  - load model from disk using existing loader
  - remove from transform graph so it isnt tried to be drawn
  -

*/

// TODO: move to some sort of render layer (not inside the abstraction layer)
typedef struct model {
  str8 debug_name;
  mesh* meshes;
  u32 mesh_count;
} model;

// ? How to tie together materials and shaders

// Three registers
// 1. low level graphics api calls "ral"
// 2. higher level render calls
// 3. simplified immediate mode API

// 3 - you don't need to know how the renderer works at all
// 2 - you need to know how the overall renderer is designed
// 1 - you need to understand graphics API specifics

/* ral.h */

// command buffer gubbins

/* --- Backends */

// struct vulkan_backend {
//   gpu_pipeline static_opaque_pipeline;
//   gpu_pipeline skinned_opaque_pipeline;
// };

/* --- Renderer layer */
/* render.h */

typedef struct renderer {
  void* backend_context;
} renderer;

bool renderer_init(renderer* ren);
void renderer_shutdown(renderer* ren);

// frontend -- these can be called from say a loop in an example, or via FFI
texture_handle texture_create(const char* debug_name, texture_desc description, const u8* data);

// Frontend Resources
void texture_data_upload(texture_handle texture);
buffer_handle buffer_create(const char* debug_name, u64 size);
bool buffer_destroy(buffer_handle buffer);
sampler_handle sampler_create();

void shader_hot_reload(const char* filepath);

// models and meshes are implemented **in terms of the above**
mesh mesh_create(geometry_data* geometry);
model_handle model_load(const char* debug_name, const char* filepath);

// Drawing

// void draw_mesh(gpu_cmd_encoder* encoder, mesh* mesh) {
//   encode_set_vertex_buffer(encoder, mesh->vertex_buffer);
//   encode_set_index_buffer(encoder, mesh->index_buffer);
//   encode_draw_indexed(encoder, mesh->index_count)
//   // vkCmdDrawIndexed
// }

// void draw_scene(arena* frame, model_darray* models, renderer* ren, camera* camera,
//                 transform_hierarchy* tfh, scene* scene) {
//                   // set the pipeline first
//                   encode_set_pipeline()
//                   // in open this sets the shader
//                   // in vulkan it sets the whole pipeline

// }