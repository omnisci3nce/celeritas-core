#pragma once

#include "defines.h"
#include "input.h"
#include "render_types.h"
#include "screenspace.h"
#include "text.h"
#include "threadpool.h"

typedef struct core {
  renderer renderer;
  threadpool threadpool;
  input_state input;
  text_system_state text;
  screenspace_state screenspace;
  model_darray* models;
} core;

// --- Lifecycle
core* core_bringup();
void core_shutdown(core* core);

void core_input_update(core* core);

bool should_window_close(core*);
void core_input_update(core* core);
void core_frame_begin(core* core);
void core_frame_end(core* core);