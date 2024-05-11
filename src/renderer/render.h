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

#include "ral_types.h"
#include "render_types.h"

bool renderer_init(renderer* ren);
void renderer_shutdown(renderer* ren);

void render_frame_begin(renderer* ren);
void render_frame_update_globals(renderer* ren);
void render_frame_end(renderer* ren);
void render_frame_draw(renderer* ren);

// ! TEMP
typedef struct camera camera;
void gfx_backend_draw_frame(renderer* ren, camera* camera, mat4 model, texture* tex);

// frontend -- these can be called from say a loop in an example, or via FFI
texture_handle texture_create(const char* debug_name, texture_desc description, const u8* data);

// Frontend Resources
// TODO: void texture_data_upload(texture_handle texture);
void texture_data_upload(texture* tex);
texture texture_data_load(const char* path, bool invert_y);
buffer_handle buffer_create(const char* debug_name, u64 size);
bool buffer_destroy(buffer_handle buffer);
sampler_handle sampler_create();

void shader_hot_reload(const char* filepath);

// models and meshes are implemented **in terms of the above**
mesh mesh_create(geometry_data* geometry);

model_handle model_load(const char* debug_name, const char* filepath);

void geo_set_vertex_colours(geometry_data* geo, vec4 colour);
