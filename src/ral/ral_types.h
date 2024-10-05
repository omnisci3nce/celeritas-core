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

typedef struct ShaderDesc {
  const char* debug_name;
  Str8 filepath;  // Where it came from
  Str8 code;      // Either GLSL or SPIRV bytecode
  bool is_spirv;
  bool is_combined_vert_frag;  // Contains both vertex and fragment stages
} ShaderDesc;

typedef ShaderDataLayout (*FN_GetBindingLayout)(void* data);

/** @brief takes a `ShaderDataLayout` without data, and puts the correct data into each binding */
typedef void (*FN_BindShaderData)(ShaderDataLayout* layout, const void* data);

// typedef struct ShaderData {
//   FN_GetBindingLayout get_layout;
//   void* data;
// } ShaderData;

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

bool GraphicsPipelineDesc_AddShaderDataLayout(GraphicsPipelineDesc* desc, ShaderDataLayout layout);

typedef struct GPU_RenderpassDesc {
  bool default_framebuffer;
  bool has_color_target;
  TextureHandle color_target;  // for now only support one
  bool has_depth_stencil;
  TextureHandle depth_stencil;
} GPU_RenderpassDesc;
