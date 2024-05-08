#pragma once

#include "defines.h"
#include "input.h"
#include "ral.h"
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
  model_darray* models;
} core;

// --- Lifecycle
core* core_bringup();
void core_shutdown(core* core);
bool should_exit(core* core);

void core_input_update(core* core);

bool should_window_close(core*);
void core_input_update(core* core);
void core_frame_begin(core* core);
void core_frame_end(core* core);