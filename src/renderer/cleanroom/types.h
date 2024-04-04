#pragma once
#include "darray.h"
#include "defines.h"
#include "maths_types.h"
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
typedef struct model bp_material;  // blinn-phong

#include "maths_types.h"

typedef enum vertex_format { VERTEX_STATIC_3D, VERTEX_SPRITE, VERTEX_COUNT } vertex_format;

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

  struct {
    vec3 position;
    vec4 colour;
    vec2 tex_coords;
    vec3 normal;
    vec4i bone_ids;     // Integer vector for bone IDs
    vec4 bone_weights;  // Weight of each bone's influence
  } animated_3d;        /** @brief vertex format for skeletal (animated) geometry in 3D */
} vertex;

KITC_DECL_TYPED_ARRAY(vertex)
KITC_DECL_TYPED_ARRAY(u32)

typedef struct geometry_data {
  vertex_format format;
  vertex_darray vertices;
  u32_darray indices;
} geometry_data;

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

/* render.h */
// frontend -- these can be called from say a loop in an example, or via FFI
texture_handle texture_create(const char* debug_name, texture_desc description, const u8* data);

void texture_data_upload(texture_handle texture);
buffer_handle buffer_create(const char* debug_name, u64 size);
bool buffer_destroy(buffer_handle buffer);

// models and meshes are implemented **in terms of the above**
mesh mesh_create(geometry_data* geometry);
model_handle model_load(const char* debug_name, const char* filepath);

/* ral.h */

enum pipeline_type {
  GRAPHICS,
  COMPUTE,
} pipeline_type;

// backend -- these are not seen by the higher-level code
typedef struct gpu_swapchain gpu_swapchain;
typedef struct gpu_device gpu_device;
typedef struct gpu_pipeline gpu_pipeline;
typedef struct gpu_cmd_encoder gpu_cmd_encoder;
typedef struct gpu_cmd_buffer gpu_cmd_buffer;  // Ready for submission

void gpu_cmd_encoder_begin();
void gpu_cmd_encoder_begin_render();
void gpu_cmd_encoder_begin_compute();

/* Actual commands that we can encode */
void encode_buffer_copy(gpu_cmd_encoder* encoder, buffer_handle src, u64 src_offset,
                        buffer_handle dst, u64 dst_offset, u64 copy_size);
void encode_clear_buffer(gpu_cmd_encoder* encoder, buffer_handle buf);
// render pass
void encode_set_vertex_buffer(gpu_cmd_encoder* encoder, buffer_handle buf);
void encode_set_index_buffer(gpu_cmd_encoder* encoder, buffer_handle buf);
void encode_draw_indexed(gpu_cmd_encoder* encoder, u64 index_count, u64* indices);

// FUTURE: compute passes

/** @brief Finish recording and return a command buffer that can be submitted to a queue */
gpu_cmd_buffer gpu_cmd_encoder_finish(gpu_cmd_encoder* encoder);

void gpu_queue_submit(gpu_cmd_buffer* buffer);

// Buffers
void gpu_buffer_create(u64 size);
void gpu_buffer_destroy(buffer_handle buffer);
void gpu_buffer_upload();
void gpu_buffer_bind(buffer_handle buffer);

// Textures
void gpu_texture_create();
void gpu_texture_destroy();
void gpu_texture_upload();

// Samplers
void gpu_sampler_create();

// command buffer gubbins

// 3. SIMA (simplified immediate mode api) / render.h
//      - dont need to worry about uploading mesh data
//      - very useful for debugging
void imm_draw_cuboid();
void imm_draw_sphere(vec3 pos, f32 radius, vec4 colour);
void imm_draw_camera_frustum();
static void imm_draw_model(
    const char* model_filepath);  // tracks internally whether the model is loaded

static void imm_draw_model(const char* model_filepath) {
  // check that model is loaded
  // if not loaded, load model and upload to gpu - LRU cache for models
  // else submit draw call
}