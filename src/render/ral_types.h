/**
 * @file ral_types.h
 * @author your name (you@domain.com)
 * @brief Struct and enum definitions for RAL
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

#define MAX_VERTEX_ATTRIBUTES 16

/* #ifndef RENDERER_TYPED_HANDLES */
CORE_DEFINE_HANDLE(buffer_handle);
CORE_DEFINE_HANDLE(texture_handle);
CORE_DEFINE_HANDLE(sampler_handle);
CORE_DEFINE_HANDLE(shader_handle);
CORE_DEFINE_HANDLE(pipeline_layout_handle);
CORE_DEFINE_HANDLE(pipeline_handle);
CORE_DEFINE_HANDLE(renderpass_handle);
#define ABSENT_MODEL_HANDLE 999999999

// --- Shaders & Bindings

typedef enum shader_visibility {
  VISIBILITY_VERTEX = 1 << 0,
  VISIBILITY_FRAGMENT = 1 << 1,
  VISIBILITY_COMPUTE = 1 << 2,
} shader_visibility;

/** @brief Describes the kind of binding a `shader_binding` is for. This changes how we create
 * backing data for it. */
typedef enum shader_binding_type {
  /**
   * @brief Binds a buffer to a shader
   * @note Vulkan: Becomes a Storage Buffer
   */
  SHADER_BINDING_BUFFER,
  SHADER_BINDING_BUFFER_ARRAY,
  SHADER_BINDING_TEXTURE,
  SHADER_BINDING_TEXTURE_ARRAY,
  SHADER_BINDING_SAMPLER,
  /**
   * @brief Binds raw data to a shader
   * @note Vulkan: Becomes a Uniform Buffer
   */
  SHADER_BINDING_BYTES,
  // TODO: Acceleration Structure
  SHADER_BINDING_COUNT
} shader_binding_type;

static const char* shader_binding_type_name[] = { "BUFFER",        "BUFFER ARRAY", "TEXTURE",
                                                  "TEXTURE ARRAY", "SAMPLER",      "BYTES",
                                                  "COUNT" };

// pub trait ShaderBindable: Clone + Copy {
//     fn bind_to(&self, context: &mut PipelineContext, index: u32);
// }

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
    struct {
      texture_handle handle;
    } texture;
  } data; /** @brief can store any kind of data that we can bind to a shader / descriptor set */
} shader_binding;

void print_shader_binding(shader_binding b);

/** @brief A list of bindings that describe what data a shader / pipeline expects
    @note This roughly correlates to a descriptor set layout in Vulkan
*/

// ? How to tie together materials and shaders

// Three registers
// 1. low level graphics api calls "ral"
// 2. higher level render calls
// 3. simplified immediate mode API

// 3 - you don't need to know how the renderer works at all
// 2 - you need to know how the overall renderer is designed
// 1 - you need to understand graphics API specifics
