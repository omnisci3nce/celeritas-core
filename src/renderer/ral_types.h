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

#include "defines.h"
#include "maths_types.h"
#include "darray.h"

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
  // gpu_texture_type tex_type;
  // gpu_texture_format format;
  // u32x2 extents;
} texture_desc;

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

#ifndef TYPED_VERTEX_ARRAY
KITC_DECL_TYPED_ARRAY(vertex)
KITC_DECL_TYPED_ARRAY(u32)
#define TYPED_VERTEX_ARRAY
#endif