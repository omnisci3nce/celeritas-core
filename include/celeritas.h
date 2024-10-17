#pragma once

// Standard library includes
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Third party dependency includes
#include <glfw3.h>

#define inlined inline  // remove inlined functions so we can generate bindings

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

_Static_assert(sizeof(bool) == 1, "type bool should be 1 byte");

_Static_assert(sizeof(u8) == 1, "type u8 should be 1 byte");
_Static_assert(sizeof(u16) == 2, "type u16 should be 2 byte");
_Static_assert(sizeof(u32) == 4, "type u32 should be 4 byte");
_Static_assert(sizeof(u64) == 8, "type u64 should be 8 byte");

_Static_assert(sizeof(i8) == 1, "type i8 should be 1 byte");
_Static_assert(sizeof(i16) == 2, "type i16 should be 2 byte");
_Static_assert(sizeof(i32) == 4, "type i32 should be 4 byte");
_Static_assert(sizeof(i64) == 8, "type i64 should be 8 byte");

_Static_assert(sizeof(f32) == 4, "type f32 should be 4 bytes");
_Static_assert(sizeof(f64) == 8, "type f64 should be 8 bytes");

_Static_assert(sizeof(ptrdiff_t) == 8, "type ptrdiff_t should be 8 bytes");

#define alignof(x) _Alignof(x)

#define threadlocal _Thread_local

// Wrap a u32 to make a type-safe "handle" or ID
#define DEFINE_HANDLE(name) \
  typedef struct name name; \
  struct name {             \
    u32 raw;                \
  }

#define KB(x) ((size_t)x * 1000)
#define MB(x) ((size_t)x * 1000 * 1000)
#define GB(x) ((size_t)x * 1000 * 1000 * 1000)

// Platform informs renderer backend (unless user overrides)
#if defined(CEL_PLATFORM_LINUX) || defined(CEL_PLATFORM_WINDOWS)
#define GPU_VULKAN 1
#elif defined(CEL_PLATFORM_MAC)
#define GPU_METAL 1
#endif

// --- Forward declare vital structures
typedef struct core core;
typedef struct renderer renderer;
typedef struct input_state input_state;
struct GLFWwindow;

// Global getters
core* get_g_core();
renderer* get_g_renderer();
struct GLFWwindow* get_window();

struct core {
  const char* app_name;
  struct GLFWwindow* window;
  renderer* renderer;
  input_state* input;
};
extern core g_core; /** @brief global `Core` that other files can use */

void core_bringup(const char* window_name, struct GLFWwindow* optional_window);
void core_shutdown();
void core_resize_viewport(int width, int height);
bool app_should_exit();

// --- Memory facilities: Allocators, helpers

// TODO: Arenas
// TODO: Pool allocator

// --- Strings

// --- Logging

// Log levels
typedef enum loglevel {
  LOG_LEVEL_FATAL = 0,
  LOG_LEVEL_ERROR = 1,
  LOG_LEVEL_WARN = 2,
  LOG_LEVEL_INFO = 3,
  LOG_LEVEL_DEBUG = 4,
  LOG_LEVEL_TRACE = 5,
} loglevel;

void log_output(char* module, loglevel level, const char* msg, ...);

#define NAMESPACED_LOGGER(module)                    \
  static inline void FATAL(const char* msg, ...) {   \
    va_list args;                                    \
    va_start(args, msg);                             \
    log_output(#module, LOG_LEVEL_FATAL, msg, args); \
    va_end(args);                                    \
  }                                                  \
  static inline void ERROR(const char* msg, ...) {   \
    va_list args;                                    \
    va_start(args, msg);                             \
    log_output(#module, LOG_LEVEL_FATAL, msg, args); \
    va_end(args);                                    \
  }                                                  \
  static inline void WARN(const char* msg, ...) {    \
    va_list args;                                    \
    va_start(args, msg);                             \
    log_output(#module, LOG_LEVEL_FATAL, msg, args); \
    va_end(args);                                    \
  }                                                  \
  static inline void INFO(const char* msg, ...) {    \
    va_list args;                                    \
    va_start(args, msg);                             \
    log_output(#module, LOG_LEVEL_FATAL, msg, args); \
    va_end(args);                                    \
  }                                                  \
  static inline void DEBUG(const char* msg, ...) {   \
    va_list args;                                    \
    va_start(args, msg);                             \
    log_output(#module, LOG_LEVEL_FATAL, msg, args); \
    va_end(args);                                    \
  }                                                  \
  static inline void TRACE(const char* msg, ...) {   \
    va_list args;                                    \
    va_start(args, msg);                             \
    log_output(#module, LOG_LEVEL_FATAL, msg, args); \
    va_end(args);                                    \
  }

// --- Maths

// Constants
#define PI 3.14159265358979323846
#define HALF_PI 1.57079632679489661923
#define TAU (2.0 * PI)

/** @brief 2D Vector */
typedef struct vec2 {
  f32 x, y;
} vec2;

/** @brief 3D Vector */
typedef struct vec3 {
  f32 x, y, z;
} vec3;

/** @brief 4D Vector */
typedef struct vec4 {
  f32 x, y, z, w;
} vec4;

/** @brief Quaternion */
typedef vec4 quat;

/** @brief 4x4 Matrix */
typedef struct mat4 {
  // TODO: use this format for more readable code: vec4 x_axis, y_axis, z_axis, w_axis;
  f32 data[16];
  
} mat4;

/** @brief 3D affine transformation */
typedef struct transform {
  vec3 position;
  quat rotation;
  vec3 scale;
  bool is_dirty;
} transform;

inlined vec3 vec3_create(f32 x, f32 y, f32 z);
inlined vec3 vec3_add(vec3 u, vec3 v);
inlined vec3 vec3_sub(vec3 u, vec3 v);
inlined vec3 vec3_mult(vec3 u, f32 s);
inlined vec3 vec3_div(vec3 u, f32 s);

// --- RAL

DEFINE_HANDLE(buf_handle);
DEFINE_HANDLE(tex_handle);
DEFINE_HANDLE(pipeline_handle);

#define MAX_VERTEX_ATTRIBUTES 16
#define MAX_SHADER_BINDINGS 16

// Backend-specific structs
typedef struct gpu_swapchain gpu_swapchain;
typedef struct gpu_compute_pipeline gpu_compute_pipeline;
typedef struct gpu_gfx_pipeline gpu_gfx_pipeline;
typedef struct gpu_encoder gpu_encoder; // Command encoder

// NOTE: Can we just use Storage buffer for everything?
// typedef enum gpu_buf_type {} gpu_buf_type;

typedef enum gpu_tex_type {
  TEXTURE_TYPE_2D,
  TEXTURE_TYPE_3D,
  TEXTURE_TYPE_2D_ARRAY,
  TEXTURE_TYPE_CUBE_MAP,
  TEXTURE_TYPE_COUNT
} gpu_tex_type;

/** @brief Texture Description - used by texture creation functions */
typedef struct texture_desc {
  gpu_tex_type tex_type;
  // GPU_TextureFormat format;
  int width, height, num_channels;
} texture_desc;

/// @strip_prefix(ATTR_)
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

typedef struct vertex_desc {
  const char* label;
  vertex_attrib_type attributes[MAX_VERTEX_ATTRIBUTES];
  u32 attribute_count;
} vertex_desc;

typedef enum shader_binding_type {
  BINDING_BYTES,
  BINDING_BUFFER,
  BINDING_BUFFER_ARRAY,
  BINDING_TEXTURE,
  BINDING_TEXTURE_ARRAY,
  BINDING_SAMPLER,
  BINDING_COUNT
} shader_binding_type;

typedef enum shader_vis {
  VISIBILITY_VERTEX = 1 << 0,
  VISIBILITY_FRAGMENT = 1 << 1,
  VISIBILITY_COMPUTE = 1 << 2,
} shader_vis;

typedef struct shader_binding {
  const char* label;
  shader_binding_type binding_type;
  shader_vis vis;
  union {
    struct {
      u32 size;
      void* data;
    } bytes;
    struct {
      buf_handle handle;
    } buffer;
    struct {
      tex_handle handle;
    } texture;
  } data;
} shader_binding;

typedef struct shader_data_layout {
  shader_binding bindings[MAX_SHADER_BINDINGS];
  size_t binding_count;
} shader_data_layout;

typedef struct shader_desc {
  // TODO
} shader_desc;

typedef enum cull_mode { CULL_BACK_FACE, CULL_FRONT_FACE } cull_mode;

typedef struct gfx_pipeline_desc {
  const char* label;
  vertex_desc vertex_desc;
  shader_desc vs;
  shader_desc fs;
  // ShaderDataLayout data_layouts[MAX_SHADER_DATA_LAYOUTS];
  // u32 data_layouts_count;
} gfx_pipeline_desc;

// --- RAL Functions
buf_handle ral_buffer_create(u64 size, const void* data);
void ral_buffer_destroy(buf_handle handle);
tex_handle ral_texture_create(texture_desc desc, bool create_view, const void* data);
void ral_texture_destroy(tex_handle handle);

// --- Containers (Forward declared as internals are unnecessary for external header)
typedef struct u32_darray u32_darray;

// --- Base Renderer types

DEFINE_HANDLE(MeshHandle);
DEFINE_HANDLE(MaterialHandle);
DEFINE_HANDLE(ModelHandle);

typedef struct geometry {
  vertex_desc vertex_format;
  void* vertex_data;
  bool has_indices;  // When this is false indexed drawing is not used
  u32_darray* indices;
} geometry;

typedef struct mesh {
  buf_handle vertex_buffer;
  buf_handle index_buffer;
  MaterialHandle material;
  geometry geometry;
  // bool is_skinned;  // false = its static
  // Armature armature;
  // bool is_uploaded;  // has the data been uploaded to the GPU
} mesh;

// --- Render primitives

geometry geo_plane(f32 x_scale, f32 z_scale, u32 tiling_u, u32 tiling_v);
geometry geo_cuboid(f32 x_scale, f32 y_scale, f32 z_scale);
geometry geo_cylinder(f32 radius, f32 height, u32 resolution);
geometry geo_cone(f32 radius, f32 height, u32 resolution);
geometry geo_uv_sphere(f32 radius, u32 north_south_lines, u32 east_west_lines);
geometry geo_ico_sphere(f32 radius, f32 n_subdivisions);

// --- Scene / Transform Hierarchy

// --- Gameplay

typedef struct Camera {
  vec3 position;
  quat orientation;
  f32 fov;
} Camera;

// --- Reference Renderer

// TODO: Filament PBR model

// --- Animation

// --- Collisions

// --- Physics

// --- Platform

// --- Audio
