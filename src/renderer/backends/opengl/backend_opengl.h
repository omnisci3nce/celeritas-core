#pragma once

#ifdef CEL_REND_BACKEND_OPENGL

#include "defines.h"
#include "maths_types.h"
#include "ral_types.h"

#define MAX_PIPELINE_UNIFORM_BUFFERS 32

typedef struct gpu_swapchain {
  u32x2 dimensions;
} gpu_swapchain;
typedef struct gpu_device {
} gpu_device;
typedef struct gpu_pipeline_layout {
  void *pad
} gpu_pipeline_layout;
typedef struct gpu_pipeline {
  u32 shader_id;
  vertex_description vertex_desc;
  buffer_handle uniform_bindings[MAX_PIPELINE_UNIFORM_BUFFERS];
} gpu_pipeline;
typedef struct gpu_renderpass {
  void *pad
} gpu_renderpass;
typedef struct gpu_cmd_encoder {
  gpu_pipeline *pipeline;
} gpu_cmd_encoder;  // Recording
typedef struct gpu_cmd_buffer {
  void *pad
} gpu_cmd_buffer;  // Ready for submission

typedef struct gpu_buffer {
  union {
    u32 vbo;
    u32 ibo;
    u32 ubo;
  } id;
  u32 vao;  // Optional
  u64 size;
} gpu_buffer;
typedef struct gpu_texture {
  void *pad
} gpu_texture;

u32 shader_create_separate(const char *vert_shader, const char *frag_shader);

void uniform_vec3f(u32 program_id, const char *uniform_name, vec3 *value);
void uniform_f32(u32 program_id, const char *uniform_name, f32 value);
void uniform_i32(u32 program_id, const char *uniform_name, i32 value);
void uniform_mat4f(u32 program_id, const char *uniform_name, mat4 *value);
#endif
