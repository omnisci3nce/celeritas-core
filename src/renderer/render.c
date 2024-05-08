#include "render.h"
#include <glfw3.h>
#include "camera.h"
#include "log.h"
#include "ral.h"

/** @brief Creates the pipelines built into Celeritas such as rendering static opaque geometry,
           debug visualisations, immediate mode UI, etc */
void default_pipelines_init(renderer* ren);

bool renderer_init(renderer* ren) {
  // INFO("Renderer init");

  // NOTE: all platforms use GLFW at the moment but thats subject to change
  glfwInit();

#if defined(CEL_REND_BACKEND_OPENGL)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#elif defined(CEL_REND_BACKEND_VULKAN)
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

  // glfw window creation
  GLFWwindow* window = glfwCreateWindow(ren->config.scr_width, ren->config.scr_height,
                                        ren->config.window_name, NULL, NULL);
  if (window == NULL) {
    // ERROR("Failed to create GLFW window\n");
    glfwTerminate();
    return false;
  }
  ren->window = window;

  glfwMakeContextCurrent(ren->window);

  DEBUG("Start backend init");

  gpu_backend_init("Celeritas Engine - Vulkan", window);
  gpu_device_create(&ren->device);  // TODO: handle errors
  gpu_swapchain_create(&ren->swapchain);

  // DEBUG("init graphics api backend");
  // if (!gfx_backend_init(ren)) {
  // FATAL("Couldnt load graphics api backend");
  // return false;
  // }

  default_pipelines_init(ren);

  // ren->blinn_phong =
  //     shader_create_separate("assets/shaders/blinn_phong.vert",
  //     "assets/shaders/blinn_phong.frag");

  // ren->skinned =
  //     shader_create_separate("assets/shaders/skinned.vert", "assets/shaders/blinn_phong.frag");

  // default_material_init();

  return true;
}
void renderer_shutdown(renderer* ren) {
  // gpu_device_destroy(ren->device);
}

void default_pipelines_init(renderer* ren) {
  // Static opaque geometry
  // graphics_pipeline_desc gfx = {
  // };
  // ren->static_opaque_pipeline = gpu_graphics_pipeline_create();
}

void render_frame_begin(renderer* ren) {}
void render_frame_end(renderer* ren) {}
void render_frame_draw(renderer* ren) {}

void gfx_backend_draw_frame(renderer* ren, camera* camera, mat4 model, texture* tex) {}

void geo_set_vertex_colours(geometry_data* geo, vec4 colour) {}