#pragma once

#include "input.h"
#include "scene.h"
#include "screenspace.h"
#include "terrain.h"
#include "text.h"
// #include "threadpool.h"

typedef struct core {
  const char* app_name;
  // foundations
  renderer renderer;
  // threadpool threadpool;
  // systems
  input_state input;
  text_system_state text;
  terrain_state terrain;
  screenspace_state screenspace;
  // data storage
  scene default_scene;
  model_darray* models;
} core;

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
