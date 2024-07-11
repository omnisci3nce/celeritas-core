#pragma once

#include "input.h"
#include "render_types.h"
#include "scene.h"
#include "screenspace.h"
#include "terrain.h"
#include "text.h"

typedef struct Core Core;


core* get_global_core();

// --- Lifecycle

/** @brief Throws error if the core cannot be instantiated */
void core_bringup();
void core_shutdown();
bool should_exit();

void frame_begin();
void frame_draw();
void frame_end();

void core_input_update();
