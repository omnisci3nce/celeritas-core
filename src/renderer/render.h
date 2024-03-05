#pragma once

#include "render_types.h"
#include "loaders.h"

// --- Lifecycle
/** @brief initialise the render system frontend */
bool renderer_init(renderer* ren);
/** @brief shutdown the render system frontend */
void renderer_shutdown(renderer* ren);

// --- Frame

void render_frame_begin(renderer* ren);
void render_frame_end(renderer* ren);

// ---
texture texture_data_load(const char* path, bool invert_y);  // #frontend
void texture_data_upload(texture* tex);                      // #backend
