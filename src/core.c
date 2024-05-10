#include "core.h"

#include <stdlib.h>

#include "glfw3.h"
#include "input.h"
#include "keys.h"
#include "log.h"
#include "render.h"
#include "render_types.h"
#include "scene.h"
// #include "threadpool.h"

#define SCR_WIDTH 1000
#define SCR_HEIGHT 1000

core g_core; /** @brief global `core` that other files can use */

inline core* get_global_core() { return &g_core; }

void core_bringup() {
  INFO("Initiate Core bringup");
  renderer_config conf = { .window_name = { "Celeritas Engine Core" },
                           .scr_width = SCR_WIDTH,
                           .scr_height = SCR_HEIGHT,
                           .clear_colour = (vec3){ .08, .08, .1 } };
  g_core.renderer.config = conf;
  g_core.renderer.backend_context = NULL;

  // threadpool_create(&c->threadpool, 6, 256);
  // threadpool_set_ctx(&c->threadpool, c);  // Gives the threadpool access to the core

  // initialise all subsystems
  if (!renderer_init(&g_core.renderer)) {
    // FATAL("Failed to start renderer");
    ERROR_EXIT("Failed to start renderer\n");
  }
  if (!input_system_init(&g_core.input, g_core.renderer.window)) {
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

  INFO("Creating default scene");
  scene_init(&g_core.default_scene);
}

#include <glfw3.h>

/* bool should_window_close(core* core) { glfwWindowShouldClose(core->renderer.window); } */
void core_input_update() { input_update(&g_core.input); }
void core_frame_begin(core* core) { render_frame_begin(&core->renderer); }
void core_frame_end(core* core) { render_frame_end(&core->renderer); }

void core_shutdown() {
  // threadpool_destroy(&core->threadpool);
  input_system_shutdown(&g_core.input);
  renderer_shutdown(&g_core.renderer);
}

bool should_exit() {
  return key_just_released(KEYCODE_ESCAPE) || glfwWindowShouldClose(g_core.renderer.window);
}

void frame_begin() { render_frame_begin(&g_core.renderer); }
void frame_draw() {}
void frame_end() { render_frame_end(&g_core.renderer); }
