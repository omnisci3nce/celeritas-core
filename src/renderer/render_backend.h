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

void clear_screen(vec3 colour);

void bind_texture(shader s, texture* tex, u32 slot);
void bind_mesh_vertex_buffer(void* backend, mesh* mesh);
void draw_primitives(cel_primitive_topology primitive, u32 start_index, u32 count);

shader shader_create_separate(const char* vert_shader, const char* frag_shader);
void set_shader(shader s);

// --- Uniforms
