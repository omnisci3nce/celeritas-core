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

// These are only the initial window dimensions
#define SCR_WIDTH 1000
#define SCR_HEIGHT 1000

Core g_core; /** @brief global `Core` that other files can use */

/** @brief Gets the global `Core` singleton */
inline Core* GetCore() { return &g_core; }

void Core_Bringup(struct GLFWwindow* optional_window) {
  INFO("Initiate Core bringup");
  memset(&g_core, 0, sizeof(Core));

  RendererConfig conf = { .window_name = { "Celeritas Engine Core" },
                          .scr_width = SCR_WIDTH,
                          .scr_height = SCR_HEIGHT,
                          .clear_colour = (Vec3){ .08, .08, .1 } };

  g_core.renderer = malloc(Renderer_GetMemReqs());
  // initialise all subsystems
  // renderer config, renderer ptr, ptr to store a window, and optional preexisting glfw window
  if (!Renderer_Init(conf, g_core.renderer, &g_core.window, optional_window)) {
    // FATAL("Failed to start renderer");
    ERROR_EXIT("Failed to start renderer\n");
  }
  if (optional_window != NULL) {
    g_core.window = optional_window;
  }

  if (!Input_Init(&g_core.input, g_core.window)) {
    // the input system needs the glfw window which is created by the renderer
    // hence the order here is important
    ERROR_EXIT("Failed to start input system\n");
  }

  size_t model_data_max = 1024 * 1024 * 1024;
  arena model_arena = arena_create(malloc(model_data_max), model_data_max);

  Model_pool model_pool = Model_pool_create(&model_arena, 256, sizeof(Model));
  g_core.models = model_pool;
  INFO("Created model pool allocator");

  // INFO("Creating default scene");
  // scene_init(&g_core.default_scene);
}

void Core_Shutdown() {
  Input_Shutdown(&g_core.input);
  Renderer_Shutdown(g_core.renderer);
  free(g_core.renderer);
}

bool ShouldExit() {
  return key_just_released(KEYCODE_ESCAPE) || glfwWindowShouldClose(g_core.window);
}

void Frame_Begin() {
  Input_Update(&g_core.input);
  Render_FrameBegin(g_core.renderer);
}
void Frame_Draw() {}
void Frame_End() { Render_FrameEnd(g_core.renderer); }

Core* get_global_core() { return &g_core; }

GLFWwindow* Core_GetGlfwWindowPtr(Core* core) { return g_core.window; }

struct Renderer* Core_GetRenderer(Core* core) { return core->renderer; }

Model* Model_Get(ModelHandle h) { return Model_pool_get(&g_core.models, h); }