#include "core.h"

#include <stdlib.h>

#include "glfw3.h"
#include "input.h"
#include "keys.h"
#include "log.h"
#include "render.h"
#include "render_types.h"
// #include "threadpool.h"

#define SCR_WIDTH 1000
#define SCR_HEIGHT 1000

core* core_bringup() {
  INFO("Initiate Core bringup");
  core* c = malloc(sizeof(core));
  renderer_config conf = { .window_name = { "Celeritas Engine Core" },
                           .scr_width = SCR_WIDTH,
                           .scr_height = SCR_HEIGHT,
                           .clear_colour = (vec3){ .08, .08, .1 } };
  c->renderer.config = conf;
  c->renderer.backend_context = NULL;

  // threadpool_create(&c->threadpool, 6, 256);
  // threadpool_set_ctx(&c->threadpool, c);  // Gives the threadpool access to the core

  // initialise all subsystems
  if (!renderer_init(&c->renderer)) {
    // FATAL("Failed to start renderer");
    ERROR_EXIT("Failed to start renderer\n");
  }
  if (!input_system_init(&c->input, c->renderer.window)) {
    // the input system needs the glfw window which is created by the renderer
    // hence the order here is important
    FATAL("Failed to start input system");
    ERROR_EXIT("Failed to start input system\n");
  }
  /*
  if (!text_system_init(&c->text)) {
    // FATAL("Failed to start text system");
    ERROR_EXIT("Failed to start text system\n");
  }
  if (!screenspace_2d_init(&c->screenspace)) {
    // FATAL("Failed to start screenspace 2d plugin");
    ERROR_EXIT("Failed to start screenspace 2d plugin\n");
  }
  */

  c->models = model_darray_new(10);

  return c;
}

#include <glfw3.h>
#include "input.h"
#include "render.h"

bool should_window_close(core* core) { glfwWindowShouldClose(core->renderer.window); }
void core_input_update(core* core) { input_update(&core->input); }
void core_frame_begin(core* core) { render_frame_begin(&core->renderer); }
void core_frame_end(core* core) { render_frame_end(&core->renderer); }

void core_shutdown(core* core) {
  // threadpool_destroy(&core->threadpool);
  input_system_shutdown(&core->input);
  renderer_shutdown(&core->renderer);
}

bool should_exit(core* core) {
  return key_just_released(KEYCODE_ESCAPE) || glfwWindowShouldClose(core->renderer.window);
}