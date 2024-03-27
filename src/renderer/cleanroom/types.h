#pragma once
#include "defines.h"

typedef int texture_handle;
typedef int buffer_handle;
typedef int model_handle;

/** @brief Texture Description - used by texture creation functions */
typedef struct texture_desc {
  // gpu_texture_type tex_type;
  // gpu_texture_format format;
  // u32x2 extents;
} texture_desc;

/* 
 - render_types.h
 - ral_types.h 
 - ral.h
 - render.h ?
*/

/* render_types */
typedef struct mesh mesh;
typedef struct model model;
typedef struct model pbr_material;
typedef struct model bp_material; // blinn-phong

// Three registers
// 1. low level graphics api calls "ral"
// 2. higher level render calls
// 3. simplified immediate mode API

/* render.h */
// frontend -- these can be called from say a loop in an example, or via FFI
texture_handle texture_create(const char* debug_name, texture_desc description);
void texture_data_upload(texture_handle texture);
buffer_handle buffer_create(const char* debug_name, u64 size);

// models and meshes are implemented **in terms of the above**
mesh mesh_create();
model_handle model_load(const char* filepath);

/* ral.h */
// backend -- these are not seen by the higher-level code
void gpu_texture_init();
void gpu_texture_upload();
void gpu_buffer_init();
void gpu_buffer_upload();

// command buffer gubbins

// 3. SIMA (simplified immediate mode api)
//      - dont need to worry about uploading mesh data
void debug_draw_cuboid();
void debug_draw_sphere();
void debug_draw_camera_frustum();
static void imm_draw_model(const char* model_filepath); // tracks internally whether the model is loaded

static void imm_draw_model(const char* model_filepath) {
  // check that model is loaded
  // if not loaded, load model and upload to gpu - LRU cache for models
  // else submit draw call
}