#include "core.h"

#include <stdlib.h>

#include "glfw3.h"
#include "input.h"
#include "keys.h"
#include "log.h"
#include "mem.h"
#include "render.h"
#include "render_types.h"
#include "scene.h"

#define SCR_WIDTH 1000
#define SCR_HEIGHT 1000

Core g_core; /** @brief global `Core` that other files can use */


struct Core {
  const char* app_name;
  Renderer renderer;
  input_state input;
  model_pool models;
};

/** @brief Gets the global `Core` singleton */
inline Core* GetCore() { return &g_core; }

void core_bringup() {
  INFO("Initiate Core bringup");
  RendererConfig conf = { .window_name = { "Celeritas Engine Core" },
                           .scr_width = SCR_WIDTH,
                           .scr_height = SCR_HEIGHT,
                           .clear_colour = (vec3){ .08, .08, .1 } };

  g_core.renderer.backend_context = NULL;

  // initialise all subsystems
  if (!Renderer_Init(conf, &g_core.renderer)) {
    // FATAL("Failed to start renderer");
    ERROR_EXIT("Failed to start renderer\n");
  }
  if (!Input_Init(&g_core.input, g_core.renderer.window)) {
    // the input system needs the glfw window which is created by the renderer
    // hence the order here is important
    ERROR_EXIT("Failed to start input system\n");
  }

  size_t model_data_max = 1024 * 1024 * 1024;
  arena model_arena = arena_create(malloc(model_data_max), model_data_max);

  model_pool model_pool = model_pool_create(&model_arena, 256, sizeof(model));
  g_core.models = model_pool;
  INFO("Created model pool allocator");

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

void frame_begin() {
  glfwPollEvents();
  render_frame_begin(&g_core.renderer);
}
void frame_draw() {}
void frame_end() { render_frame_end(&g_core.renderer); }
