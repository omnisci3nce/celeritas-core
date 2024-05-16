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

typedef struct render_ctx {
    mat4 view;
    mat4 projection;
} render_ctx;

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

/**
 * @brief Creates buffers and returns a struct that holds handles to our resources
 * 
 * @param geometry 
 * @param free_on_upload frees the CPU-side vertex/index data stored in geometry_data when we successfully upload
                         that data to the GPU-side buffer
 * @return mesh 
 */
mesh mesh_create(geometry_data* geometry, bool free_on_upload);

void draw_mesh(mesh* mesh, mat4* model);//, mat4* view, mat4* proj); // TODO: material

model_handle model_load(const char* debug_name, const char* filepath);

void geo_free_data(geometry_data* geo);
void geo_set_vertex_colours(geometry_data* geo, vec4 colour);

vertex_description static_3d_vertex_description();
