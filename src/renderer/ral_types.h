/**
 * @file ral_types.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-27
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "darray.h"
#include "defines.h"
#include "maths_types.h"

#ifndef RENDERER_TYPED_HANDLES
CORE_DEFINE_HANDLE(buffer_handle);
CORE_DEFINE_HANDLE(texture_handle);
CORE_DEFINE_HANDLE(sampler_handle);
CORE_DEFINE_HANDLE(shader_handle);
CORE_DEFINE_HANDLE(model_handle);
#define ABSENT_MODEL_HANDLE 999999999
#define RENDERER_TYPED_HANDLES
#endif

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

/** @brief Texture Description - used by texture creation functions */
typedef struct texture_desc {
  gpu_texture_type tex_type;
  gpu_texture_format format;
  u32x2 extents;
} texture_desc;

typedef enum gpu_buffer_type {
  CEL_BUFFER_DEFAULT,  // on Vulkan this would be a storage buffer?
  CEL_BUFFER_VERTEX,
  CEL_BUFFER_INDEX,
  CEL_BUFFER_UNIFORM,
  CEL_BUFFER_COUNT
} gpu_buffer_type;

typedef enum gpu_buffer_flag {
  CEL_BUFFER_FLAG_CPU = 1 << 0,
  CEL_BUFFER_FLAG_GPU = 1 << 1,
  CEL_BUFFER_FLAG_STORAGE = 1 << 2,
  CEL_BUFFER_FLAG_COUNT
} gpu_buffer_flag;
typedef u32 gpu_buffer_flags;

typedef enum vertex_format {
  VERTEX_STATIC_3D,
  VERTEX_SPRITE,
  VERTEX_SKINNED,
  VERTEX_COLOURED_STATIC_3D,
  VERTEX_RAW_POS_COLOUR,
  VERTEX_COUNT
} vertex_format;

typedef union vertex {
  struct {
    vec3 position;
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

  struct {
    vec3 position;
    vec2 tex_coords;
    vec3 normal;
    vec4 colour;
  } coloured_static_3d; /** @brief vertex format used for debugging */

  struct {
    vec2 position;
    vec3 colour;
  } raw_pos_colour;
} vertex;

#ifndef TYPED_VERTEX_ARRAY
KITC_DECL_TYPED_ARRAY(vertex)
KITC_DECL_TYPED_ARRAY(u32)
#define TYPED_VERTEX_ARRAY
#endif

// TEMP
typedef struct custom_vertex {
  vec2 pos;
  vec3 color;
} custom_vertex;

// Vertex attributes
typedef enum vertex_attrib_type {
  ATTR_F32,
  ATTR_F32x2,
  ATTR_F32x3,
  ATTR_F32x4,
  ATTR_U32,
  ATTR_U32x2,
  ATTR_U32x3,
  ATTR_U32x4,
  ATTR_I32,
  ATTR_I32x2,
  ATTR_I32x3,
  ATTR_I32x4,
} vertex_attrib_type;

typedef enum shader_visibility {
  VISIBILITY_VERTEX = 1 << 0,
  VISIBILITY_FRAGMENT = 1 << 1 ,
  VISIBILITY_COMPUTE = 1 << 2,
} shader_visibility;

typedef enum shader_binding_type {
  SHADER_BINDING_BUFFER,
  SHADER_BINDING_TEXTURE,
  SHADER_BINDING_BYTES,
  SHADER_BINDING_COUNT
} shader_binding_type;

typedef struct shader_binding {
  const char* label;
  shader_binding_type type;
  shader_visibility vis;
  bool stores_data; /** @brief if this is true then the shader binding has references to live data,
                               if false then its just being used to describe a layout and .data
                       should be zeroed */
  union {
    struct {
      buffer_handle handle;
    } buffer;
    struct {
      void* data;
      size_t size;
    } bytes;
  } data;
} shader_binding;

#define MAX_LAYOUT_BINDINGS 8

/** @brief A list of bindings that describe what data a shader / pipeline expects 
    @note This roughly correlates to a descriptor set layout in Vulkan
*/
typedef struct shader_data_layout {
  char* name;
  shader_binding bindings[MAX_LAYOUT_BINDINGS];
  u32 bindings_count;
} shader_data_layout;

typedef struct shader_data {
  shader_data_layout (*shader_data_get_layout)(void* data);
  void* data;
} shader_data;

/*
  Usage:
    1. When we create the pipeline, we must call a function that return a layout without .data fields
    2. When binding 
*/

typedef enum gpu_cull_mode { CULL_BACK_FACE, CULL_FRONT_FACE, CULL_COUNT } gpu_cull_mode;

// ? How to tie together materials and shaders

// Three registers
// 1. low level graphics api calls "ral"
// 2. higher level render calls
// 3. simplified immediate mode API

// 3 - you don't need to know how the renderer works at all
// 2 - you need to know how the overall renderer is designed
// 1 - you need to understand graphics API specifics
