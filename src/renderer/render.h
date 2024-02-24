#pragma once

#include "render_types.h"

// --- Lifecycle
/** @brief initialise the render system frontend */
bool renderer_init(renderer* ren);
/** @brief shutdown the render system frontend */
void renderer_shutdown(renderer* ren);

// --- Frame

void render_frame_begin(renderer* ren);
void render_frame_end(renderer* ren);

// ---