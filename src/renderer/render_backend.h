/**
 * @brief
 */
#pragma once

#include "maths_types.h"
#include "render_types.h"

/// --- Lifecycle
bool gfx_backend_init(renderer* ren);
void gfx_backend_shutdown(renderer* ren);

void clear_screen(vec3 colour);

// --- Uniforms
