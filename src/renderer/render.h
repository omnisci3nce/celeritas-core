#pragma once

#include "camera.h"
#include "loaders.h"
#include "render_types.h"

// --- Lifecycle
/** @brief initialise the render system frontend */
bool renderer_init(renderer* ren);
/** @brief shutdown the render system frontend */
void renderer_shutdown(renderer* ren);

// --- Frame

void render_frame_begin(renderer* ren);
void render_frame_end(renderer* ren);

// --- models meshes
void model_upload_meshes(renderer* ren, model* model);
void draw_model(renderer* ren, camera* camera, model* model, transform tf);
void draw_mesh(renderer* ren, mesh* mesh, transform tf, material* mat, mat4* view, mat4* proj);

// ---
texture texture_data_load(const char* path, bool invert_y);  // #frontend
void texture_data_upload(texture* tex);                      // #backend

// --- Uniforms

/** @brief upload a vec3 of f32 to a uniform */
void uniform_vec3f(u32 program_id, const char *uniform_name, vec3 *value);
/** @brief upload a single f32 to a uniform */
void uniform_f32(u32 program_id, const char *uniform_name, f32 value);
/** @brief upload a integer to a uniform */
void uniform_i32(u32 program_id, const char *uniform_name, i32 value);
/** @brief upload a mat4 of f32 to a uniform */
void uniform_mat4f(u32 program_id, const char *uniform_name, mat4 *value);