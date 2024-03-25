/**
 * @brief Renderer backend
 */
#pragma once

#include "maths_types.h"
#include "render_types.h"

/// --- Lifecycle

/** @brief Initialise the graphics API backend */
bool gfx_backend_init(renderer* ren);
void gfx_backend_shutdown(renderer* ren);

void gfx_backend_draw_frame(renderer* ren);

void clear_screen(vec3 colour);

void bind_texture(shader s, texture* tex, u32 slot);
void bind_mesh_vertex_buffer(void* backend, mesh* mesh);
void draw_primitives(cel_primitive_topology primitive, u32 start_index, u32 count);

shader shader_create_separate(const char* vert_shader, const char* frag_shader);
void set_shader(shader s);

// --- Uniforms

/** @brief upload a vec3 of f32 to a uniform */
void uniform_vec3f(u32 program_id, const char* uniform_name, vec3* value);
/** @brief upload a single f32 to a uniform */
void uniform_f32(u32 program_id, const char* uniform_name, f32 value);
/** @brief upload a integer to a uniform */
void uniform_i32(u32 program_id, const char* uniform_name, i32 value);
/** @brief upload a mat4 of f32 to a uniform */
void uniform_mat4f(u32 program_id, const char* uniform_name, mat4* value);