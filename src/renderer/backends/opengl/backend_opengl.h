#pragma once

#define CEL_REND_BACKEND_OPENGL

#if defined(CEL_REND_BACKEND_OPENGL)

#include "defines.h"
#include "maths_types.h"

typedef struct gpu_swapchain {
  u32x2 dimensions;
} gpu_swapchain;
typedef struct gpu_device {} gpu_device;
typedef struct gpu_pipeline_layout {} gpu_pipeline_layout;
typedef struct gpu_pipeline {} gpu_pipeline;
typedef struct gpu_renderpass {} gpu_renderpass;
typedef struct gpu_cmd_encoder {} gpu_cmd_encoder;  // Recording
typedef struct gpu_cmd_buffer {} gpu_cmd_buffer;    // Ready for submission

typedef struct gpu_buffer {
  union {
    u32 vbo;
    u32 ibo;
  } id;
} gpu_buffer;
typedef struct gpu_texture {} gpu_texture;

#endif