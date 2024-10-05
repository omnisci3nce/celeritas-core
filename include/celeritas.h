#pragma once

// Standard library includes
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
#define CEL_REND_BACKEND_VULKAN 1
#elif defined(CEL_PLATFORM_MAC)
#define CEL_REND_BACKEND_METAL 1
#endif

// --- Forward declare vital structures
typedef struct Core Core;
typedef struct Renderer Renderer;
typedef struct InputState InputState;
struct GLFWwindow;

// Global getters
Core* GetGlobalCore();
Renderer* GetRenderer();
struct GLFWwindow* GetWindow();

struct Core {
  const char* app_name;
  struct GLFWwindow* window;
  Renderer* renderer;
  InputState* input;
};
extern Core g_core; /** @brief global `Core` that other files can use */

void Core_Bringup(const char* window_name);
void Core_Shutdown();
bool should_exit();

// --- Memory facilities: Allocators, helpers

// TODO: Arenas
// TODO: Pool allocator

// --- Strings

// --- Maths

/** @brief 3D Vector */
typedef struct Vec3 {
  f32 x, y, z;
} Vec3;

/** @brief 4D Vector */
typedef struct Vec4 {
  f32 x, y, z, w;
} Vec4;

/** @brief Quaternion */
typedef Vec4 Quat;

inlined Vec3 Vec3_Create(f32 x, f32 y, f32 z);
inlined Vec3 Vec3_Add(Vec3 u, Vec3 v);
inlined Vec3 Vec3_Sub(Vec3 u, Vec3 v);
inlined Vec3 Vec3_Mult(Vec3 u, f32 s);
inlined Vec3 Vec3_Div(Vec3 u, f32 s);

// --- RAL

DEFINE_HANDLE(BufHandle);
DEFINE_HANDLE(TexHandle);
DEFINE_HANDLE(PipelineHandle);

#define MAX_VERTEX_ATTRIBUTES 16
#define MAX_SHADER_BINDINGS 16

// Backend-specific structs
typedef struct GPU_Swapchain GPU_Swapchain;
typedef struct GPU_Pipeline GPU_Pipeline;
typedef struct GPU_CmdEncoder GPU_CmdEncoder;

// NOTE: Can we just use Storage buffer for everything?
// typedef enum GPU_BufferType {} GPU_BufferType;

typedef enum GPU_TextureType {
  TEXTURE_TYPE_2D,
  TEXTURE_TYPE_3D,
  TEXTURE_TYPE_2D_ARRAY,
  TEXTURE_TYPE_CUBE_MAP,
  TEXTURE_TYPE_COUNT
} GPU_TextureType;

/** @brief Texture Description - used by texture creation functions */
typedef struct TextureDesc {
  GPU_TextureType tex_type;
  // GPU_TextureFormat format;
  int width, height, num_channels;
} TextureDesc;

/// @strip_prefix(ATTR_)
typedef enum VertexAttribType {
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
} VertexAttribType;

typedef struct VertexDesc {
  const char* label;
  VertexAttribType attributes[MAX_VERTEX_ATTRIBUTES];
  u32 attribute_count;
} VertexDesc;

typedef struct ShaderDesc {
} ShaderDesc;

typedef enum ShaderBindingKind {
  BINDING_BYTES,
  BINDING_BUFFER,
  BINDING_BUFFER_ARRAY,
  BINDING_TEXTURE,
  BINDING_TEXTURE_ARRAY,
  BINDING_SAMPLER,
  BINDING_COUNT
} ShaderBindingKind;

typedef enum ShaderVisibility {
  VISIBILITY_VERTEX = 1 << 0,
  VISIBILITY_FRAGMENT = 1 << 1,
  VISIBILITY_COMPUTE = 1 << 2,
} ShaderVisibility;

typedef struct ShaderBinding {
  const char* label;
  ShaderBindingKind kind;
  ShaderVisibility vis;
  union {
    struct {
      u32 size;
      void* data;
    } bytes;
    struct {
      BufHandle handle;
    } buffer;
    struct {
      TexHandle handle;
    } texture;
  } data;
} ShaderBinding;

typedef struct ShaderDataLayout {
  ShaderBinding bindings[MAX_SHADER_BINDINGS];
  size_t binding_count;
} ShaderDataLayout;

typedef enum CullMode { CULL_BACK_FACE, CULL_FRONT_FACE } CullMode;

typedef struct GraphicsPipelineDesc {
  const char* label;
  VertexDesc vertex_desc;
  ShaderDesc vs;
  ShaderDesc fs;
  // ShaderDataLayout data_layouts[MAX_SHADER_DATA_LAYOUTS];
  // u32 data_layouts_count;
} GraphicsPipelineDesc;

// --- RAL Functions
BufHandle GPU_BufferCreate(u64 size, const void* data);
void GPU_BufferDestroy(BufHandle handle);
TexHandle GPU_TextureCreate(TextureDesc desc, bool create_view, const void* data);
void GPU_TextureDestroy(TexHandle handle);

// --- Containers (Forward declared as internals are unnecessary for external header)
typedef struct u32_darray u32_darray;

// --- Base Renderer

DEFINE_HANDLE(MeshHandle);
DEFINE_HANDLE(MaterialHandle);
DEFINE_HANDLE(ModelHandle);

typedef struct Geometry {
  VertexDesc vertex_format;
  void* vertex_data;
  bool has_indices;  // When this is false indexed drawing is not used
  u32_darray* indices;
} Geometry;

typedef struct Mesh {
  BufHandle vertex_buffer;
  BufHandle index_buffer;
  MaterialHandle material;
  Geometry geometry;
  // bool is_skinned;  // false = its static
  // Armature armature;
  // bool is_uploaded;  // has the data been uploaded to the GPU
} Mesh;

// --- Render primitives

Geometry Geo_CreatePlane(f32 x_scale, f32 z_scale, u32 tiling_u, u32 tiling_v);
Geometry Geo_CreateCuboid(f32 x_scale, f32 y_scale, f32 z_scale);
Geometry Geo_CreateCylinder(f32 radius, f32 height, u32 resolution);
Geometry Geo_CreateCone(f32 radius, f32 height, u32 resolution);
Geometry Geo_CreateUVsphere(f32 radius, u32 north_south_lines, u32 east_west_lines);
Geometry Geo_CreateIcosphere(f32 radius, f32 n_subdivisions);

// --- Scene / Transform Hierarchy

// --- Gameplay

typedef struct Camera {
  Vec3 position;
  Quat orientation;
  f32 fov;
} Camera;

// --- Animation

// --- Collisions

// --- Physics
