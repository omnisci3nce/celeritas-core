#pragma once
#include "defines.h"

#if defined(CEL_REND_BACKEND_OPENGL)

#include "maths_types.h"
#include "ral_impl.h"
#include "ral_types.h"

#define MAX_PIPELINE_UNIFORM_BUFFERS 32

#define OPENGL_DEFAULT_FRAMEBUFFER 0

typedef struct GPU_Swapchain {
  u32x2 dimensions;
} GPU_Swapchain;

typedef struct GPU_Device {
  u32 pad;
} GPU_Device;

typedef struct GPU_PipelineLayout {
  void* pad;
} GPU_PipelineLayout;

typedef struct GPU_Pipeline {
  u32 shader_id;
  GPU_Renderpass* renderpass;
  VertexDescription vertex_desc;
  BufferHandle uniform_bindings[MAX_PIPELINE_UNIFORM_BUFFERS];
  u32 uniform_count;
  bool wireframe;
} GPU_Pipeline;

typedef struct GPU_Renderpass {
  u32 fbo;
  GPU_RenderpassDesc description;
} GPU_Renderpass;

typedef struct GPU_CmdEncoder {
  GPU_Pipeline* pipeline;
} GPU_CmdEncoder;  // Recording

typedef struct GPU_CmdBuffer {
  void* pad;
} GPU_CmdBuffer;  // Ready for submission

typedef struct GPU_Buffer {
  union {
    u32 vbo;
    u32 ibo;
    u32 ubo;
  } id;
  union {
    u32 vao;
    u32 ubo_binding_point;
  };  // Optional
  char* name;
  u64 size;
} GPU_Buffer;

typedef struct GPU_Texture {
  u32 id;
  GPU_TextureType type;
} GPU_Texture;

typedef struct opengl_support {
  u32 pad;
} opengl_support;

void uniform_vec3f(u32 program_id, const char* uniform_name, Vec3* value);
void uniform_f32(u32 program_id, const char* uniform_name, f32 value);
void uniform_i32(u32 program_id, const char* uniform_name, i32 value);
void uniform_mat4f(u32 program_id, const char* uniform_name, Mat4* value);

typedef enum GlCommandType {
  GLCMD_DRAW,
  GLCMD_DRAW_INDEXED,
  GLCMD_BIND_VBUF,
  GLCMD_BIND_IBUF,
  GLCMD_SET_PROGRAM,
} GlCommandType;

typedef struct GlCommand {
  GlCommandType cmd_type;
  union {
    struct {
      PrimitiveTopology topology;
      u32 start_vertex;
      u32 vertex_count;
      // TODO: instance
    } draw;
    struct {
      PrimitiveTopology topology;
      u32 index_count;
    } draw_indexed;
    struct {
      u32 buffer_id;
    } bind_vbuf;
    struct {
      u32 buffer_id;
    } bind_ibuf;
    struct {
      u32 program_id;
    } set_program;
  } data;
} GlCommand;

#endif
