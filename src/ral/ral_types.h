#pragma once
#include "darray.h"
#include "defines.h"
#include "maths_types.h"
#include "str.h"

// --- Max size constants
#define MAX_SHADER_DATA_LAYOUTS 8
#define MAX_SHADER_BINDINGS 8
#define MAX_BUFFERS 256
#define MAX_TEXTURES 256
#define MAX_PIPELINES 128
#define MAX_RENDERPASSES 128
#define MAX_VERTEX_ATTRIBUTES 16

// --- Handle types
CORE_DEFINE_HANDLE(BufferHandle);
CORE_DEFINE_HANDLE(TextureHandle);
CORE_DEFINE_HANDLE(SamplerHandle);
CORE_DEFINE_HANDLE(ShaderHandle);
CORE_DEFINE_HANDLE(PipelineLayoutHandle);
CORE_DEFINE_HANDLE(PipelineHandle);
CORE_DEFINE_HANDLE(RenderpassHandle);
#define INVALID_TEX_HANDLE ((TextureHandle){ .raw = 9999981 })

// --- Buffers
typedef enum GPU_BufferType {
  BUFFER_DEFAULT,  // on Vulkan this would be a storage buffer?
  BUFFER_VERTEX,
  BUFFER_INDEX,
  BUFFER_UNIFORM,
  BUFFER_COUNT
} GPU_BufferType;

static const char* buffer_type_names[] = {
  "RAL Buffer Default", "RAL Buffer Vertex", "RAL Buffer Index",
  "RAL Buffer Uniform", "RAL Buffer Count",
};

typedef enum GPU_BufferFlag {
  BUFFER_FLAG_CPU = 1 << 0,
  BUFFER_FLAG_GPU = 1 << 1,
  BUFFER_FLAG_STORAGE = 1 << 2,
  BUFFER_FLAG_COUNT
} GPU_BufferFlag;
typedef u32 GPU_BufferFlags;

// --- Textures
typedef enum GPU_TextureType {
  TEXTURE_TYPE_2D,
  TEXTURE_TYPE_3D,
  TEXTURE_TYPE_2D_ARRAY,
  TEXTURE_TYPE_CUBE_MAP,
  TEXTURE_TYPE_COUNT
} GPU_TextureType;

static const char* texture_type_names[] = {
  "RAL Texture 2D",      "RAL Texture 3D",    "RAL Texture 2D Array",
  "RAL Texture Cubemap", "RAL Texture Count",
};

typedef enum GPU_TextureFormat {
  TEXTURE_FORMAT_8_8_8_8_RGBA_UNORM,
  TEXTURE_FORMAT_8_8_8_RGB_UNORM,
  TEXTURE_FORMAT_DEPTH_DEFAULT,
  TEXTURE_FORMAT_COUNT
} GPU_TextureFormat;

/** @brief Texture Description - used by texture creation functions */
typedef struct TextureDesc {
  GPU_TextureType tex_type;
  GPU_TextureFormat format;
  u32x2 extents;
  u32 num_channels;
} TextureDesc;

// --- Vertices

typedef enum VertexFormat {
  VERTEX_STATIC_3D,
  VERTEX_SPRITE,
  VERTEX_SKINNED,
  VERTEX_COLOURED_STATIC_3D,
  VERTEX_RAW_POS_COLOUR,
  VERTEX_POS_ONLY,
  VERTEX_COUNT
} VertexFormat;

typedef union Vertex {
  struct {
    Vec3 position;
    Vec3 normal;
    Vec2 tex_coords;
  } static_3d; /** @brief standard vertex format for static geometry in 3D */

  struct {
    Vec2 position;
    Vec4 colour;
    Vec2 tex_coords;
  } sprite; /** @brief vertex format for 2D sprites or quads */

  struct {
    Vec3 position;
    Vec4 colour;
    Vec2 tex_coords;
    Vec3 normal;
    Vec4i bone_ids;     // Integer vector for bone IDs
    Vec4 bone_weights;  // Weight of each bone's influence
  } skinned_3d;         /** @brief vertex format for skeletal (animated) geometry in 3D */

  struct {
    Vec3 position;
    Vec2 tex_coords;
    Vec3 normal;
    Vec4 colour;
  } coloured_static_3d; /** @brief vertex format used for debugging */

  struct {
    Vec2 position;
    Vec3 colour;
  } raw_pos_colour;

  struct {
    Vec3 position;
  } pos_only;
} Vertex;

#ifndef TYPED_VERTEX_ARRAY
KITC_DECL_TYPED_ARRAY(Vertex);
KITC_DECL_TYPED_ARRAY(u32)
#define TYPED_VERTEX_ARRAY
#endif

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

typedef struct VertexDescription {
  const char* debug_label;
  const char* attr_names[MAX_VERTEX_ATTRIBUTES];
  VertexAttribType attributes[MAX_VERTEX_ATTRIBUTES];
  u32 attributes_count;
  // size_t stride;
  bool use_full_vertex_size;
} VertexDescription;

// --- Shaders
typedef enum PipelineKind {
  PIPELINE_GRAPHICS,
  PIPELINE_COMPUTE,
} PipelineKind;

typedef enum ShaderVisibility {
  VISIBILITY_VERTEX = 1 << 0,
  VISIBILITY_FRAGMENT = 1 << 1,
  VISIBILITY_COMPUTE = 1 << 2,
} ShaderVisibility;

typedef struct ShaderDesc {
  const char* debug_name;
  Str8 filepath;  // Where it came from
  Str8 code;      // Either GLSL or SPIRV bytecode
  bool is_spirv;
  bool is_combined_vert_frag;  // Contains both vertex and fragment stages
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
      BufferHandle handle;
    } buffer;
    struct {
      TextureHandle handle;
    } texture;
  } data;
} ShaderBinding;

typedef struct ShaderDataLayout {
  ShaderBinding bindings[MAX_SHADER_BINDINGS];
  size_t binding_count;
} ShaderDataLayout;

typedef ShaderDataLayout (*FN_GetBindingLayout)(void* data);

/** @brief takes a `ShaderDataLayout` without data, and puts the correct data into each binding */
typedef void (*FN_BindShaderData)(ShaderDataLayout* layout, const void* data);

// typedef struct ShaderData {
//   FN_GetBindingLayout get_layout;
//   void* data;
// } ShaderData;

// --- Miscellaneous

#define TOPOLOGY_SHORT_NAMES

typedef enum PrimitiveTopology {
#ifdef TOPOLOGY_SHORT_NAMES
  CEL_POINT,
  CEL_LINE,
  CEL_LINE_STRIP,
  CEL_TRI,
  CEL_TRI_STRIP,
  PRIMITIVE_TOPOLOGY_COUNT
#else
  PRIMITIVE_TOPOLOGY_POINT,
  PRIMITIVE_TOPOLOGY_LINE,
  PRIMITIVE_TOPOLOGY_LINE_STRIP,
  PRIMITIVE_TOPOLOGY_TRIANGLE,
  PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
  PRIMITIVE_TOPOLOGY_COUNT
#endif
} PrimitiveTopology;

typedef enum CullMode { CULL_BACK_FACE, CULL_FRONT_FACE, CULL_COUNT } CullMode;

typedef enum Winding { WINDING_CCW, WINDING_CW } Winding;

// based on https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDepthFunc.xhtml
typedef enum CompareFunc {
  COMPARE_NEVER,
  COMPARE_LESS,
  COMPARE_EQUAL,
  COMPARE_LESS_EQUAL,
  COMPARE_GREATER,
  COMPARE_NOT_EQUAL,
  COMPARE_GREATER_EQUAL,
  COMPARE_ALWAYS,
  COMPARE_COUNT
} CompareFunc;

typedef struct GraphicsPipelineDesc {
  const char* debug_name;
  VertexDescription vertex_desc;
  ShaderDesc vs; /** @brief Vertex shader stage */
  ShaderDesc fs; /** @brief Fragment shader stage */

  // Roughly equivalent to a descriptor set layout each. each layout can have multiple bindings
  // examples:
  // - uniform buffer representing view projection matrix
  // - texture for shadow map
  ShaderDataLayout data_layouts[MAX_SHADER_DATA_LAYOUTS];
  u32 data_layouts_count;

  bool wireframe;
  bool depth_test;
} GraphicsPipelineDesc;

bool GraphicsPipelineDesc_AddShaderDataLayout(GraphicsPipelineDesc* desc, ShaderDataLayout layout);

typedef struct GPU_RenderpassDesc {
  bool default_framebuffer;
  bool has_color_target;
  TextureHandle color_target;  // for now only support one
  bool has_depth_stencil;
  TextureHandle depth_stencil;
} GPU_RenderpassDesc;
