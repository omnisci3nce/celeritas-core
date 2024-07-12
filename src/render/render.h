/**
 * @file render.h
 * @author your name (you@domain.com)
 * @brief Renderer frontend
 * @version 0.1
 * @date 2024-03-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "file.h"
#include "ral_types.h"
#include "render_types.h"

/** @brief configuration passed to the renderer at init time */
typedef struct renderer_config {
  char window_name[256];
  u32 scr_width, scr_height;
  vec3 clear_colour; /** colour that the screen gets cleared to every frame */
} renderer_config;

typedef struct renderer {
  struct GLFWwindow* window;
  void* backend_context;
  renderer_config config;
  gpu_device device;
  gpu_swapchain swapchain;
  gpu_renderpass default_renderpass;
  gpu_pipeline static_opaque_pipeline;
  bool frame_aborted;
  struct resource_pools* resource_pools;
} renderer;

bool renderer_init(renderer* ren);
void renderer_shutdown(renderer* ren);

void render_frame_begin(renderer* ren);
void render_frame_update_globals(renderer* ren);
void render_frame_end(renderer* ren);
void render_frame_draw(renderer* ren);

// ! TEMP
typedef struct camera camera;
void gfx_backend_draw_frame(renderer* ren, camera* camera, mat4 model, texture* tex);

typedef struct render_ctx {
  mat4 view;
  mat4 projection;
} render_ctx;

// frontend -- these can be called from say a loop in an example, or via FFI
texture_handle texture_create(const char* debug_name, texture_desc description, const u8* data);

// Frontend Resources
texture_data texture_data_load(const char* path, bool invert_y);

/**
 * @brief
 *
 * @param data
 * @param free_on_upload frees the CPU-side pixel data stored in `data`
 * @return texture_handle
 */
texture_handle texture_data_upload(texture_data data, bool free_on_upload);

/** @brief load all of the texture for a PBR material and returns an unnamed material */
material pbr_material_load(char* albedo_path, char* normal_path, bool metal_roughness_combined,
                           char* metallic_path, char* roughness_map, char* ao_map);


/**
 * @brief Creates buffers and returns a struct that holds handles to our resources
 *
 * @param geometry
 * @param free_on_upload frees the CPU-side vertex/index data stored in `geometry` when we
 successfully upload that data to the GPU-side buffer
 * @return mesh
 */
mesh mesh_create(geometry_data* geometry, bool free_on_upload);
void mesh_delete(mesh* mesh); // TODO

void draw_mesh(mesh* mesh, mat4* model, camera* cam);

model_handle model_load(const char* debug_name, const char* filepath);
