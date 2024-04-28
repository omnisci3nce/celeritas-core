/**
 * @file ral.h
 * @author your name (you@domain.com)
 * @brief Render Abstraction Layer
 * @details API that a graphics backend *must* implement
 * @version 0.1
 * @date 2024-03-31
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "ral_types.h"
#include "defines.h"
#include "str.h"
#include "buf.h"

// Unrelated forward declares
typedef struct arena arena;
struct GLFWwindow;

// Forward declare structs
typedef struct gpu_swapchain gpu_swapchain;
typedef struct gpu_device gpu_device;
typedef struct gpu_pipeline gpu_pipeline;
typedef struct gpu_renderpass gpu_renderpass;
typedef struct gpu_cmd_encoder gpu_cmd_encoder;  // Recording
typedef struct gpu_cmd_buffer gpu_cmd_buffer;    // Ready for submission

typedef enum pipeline_kind {
  PIPELINE_GRAPHICS,
  PIPELINE_COMPUTE,
} pipeline_kind;

typedef struct shader_desc {
  const char* debug_name;
  str8 filepath;  // where it came from
  str8 glsl;      // contents
} shader_desc;

struct graphics_pipeline_desc {
  shader_desc vs; /** @brief Vertex shader stage */
  shader_desc fs; /** @brief Fragment shader stage */
};

// --- Lifecycle functions

bool gpu_backend_init(const char* window_name, struct GLFWwindow* window);
void gpu_backend_shutdown();

bool gpu_device_create(gpu_device* out_device);
void gpu_device_destroy();

gpu_renderpass* gpu_renderpass_create();
void gpu_renderpass_destroy(gpu_renderpass* pass);

gpu_pipeline* gpu_graphics_pipeline_create(struct graphics_pipeline_desc description);
void gpu_pipeline_destroy(gpu_pipeline* pipeline);

bool gpu_swapchain_create(gpu_swapchain* out_swapchain);
void gpu_swapchain_destroy();

void gpu_cmd_encoder_begin();
void gpu_cmd_encoder_begin_render();
void gpu_cmd_encoder_begin_compute();

/* Actual commands that we can encode */
void encode_buffer_copy(gpu_cmd_encoder* encoder, buffer_handle src, u64 src_offset,
                        buffer_handle dst, u64 dst_offset, u64 copy_size);
void encode_clear_buffer(gpu_cmd_encoder* encoder, buffer_handle buf);
void encode_set_pipeline(gpu_cmd_encoder* encoder, gpu_pipeline* pipeline);

/** @brief Upload CPU-side data as array of bytes to a GPU buffer */
void buffer_upload_bytes(buffer_handle gpu_buf, bytebuffer cpu_buf, u64 offset, u64 size);

// render pass
void encode_set_vertex_buffer(gpu_cmd_encoder* encoder, buffer_handle buf);
void encode_set_index_buffer(gpu_cmd_encoder* encoder, buffer_handle buf);
void encode_set_bind_group();
void encode_draw(gpu_cmd_encoder* encoder);
void encode_draw_indexed(gpu_cmd_encoder* encoder, u64 index_count);

// FUTURE: compute passes

/** @brief Finish recording and return a command buffer that can be submitted to a queue */
gpu_cmd_buffer gpu_cmd_encoder_finish(gpu_cmd_encoder* encoder);

void gpu_queue_submit(gpu_cmd_buffer* buffer);

// Buffers
void gpu_buffer_create(u64 size);
void gpu_buffer_destroy(buffer_handle buffer);
void gpu_buffer_upload();
void gpu_buffer_bind(buffer_handle buffer);

// Textures
void gpu_texture_create();
void gpu_texture_destroy();
void gpu_texture_upload();

// Samplers
void gpu_sampler_create();

// --- Vertex formats
bytebuffer vertices_as_bytebuffer(arena* a, vertex_format format, vertex_darray* vertices);

// TODO: Bindgroup texture samplers / shader resources