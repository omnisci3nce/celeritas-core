#include "render.h"
#include <glfw3.h>
#include "camera.h"

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

  // DEBUG("init graphics api backend");
  // if (!gfx_backend_init(ren)) {
  // FATAL("Couldnt load graphics api backend");
  // return false;
  // }

  // ren->blinn_phong =
  //     shader_create_separate("assets/shaders/blinn_phong.vert",
  //     "assets/shaders/blinn_phong.frag");

  // ren->skinned =
  //     shader_create_separate("assets/shaders/skinned.vert", "assets/shaders/blinn_phong.frag");

  // default_material_init();

  return true;
}
void renderer_shutdown(renderer* ren) {}

void render_frame_begin(renderer* ren) {}
void render_frame_end(renderer* ren) {}
void render_frame_draw(renderer* ren) {}

void gfx_backend_draw_frame(renderer* ren, camera* camera, mat4 model, texture* tex) {}

void geo_set_vertex_colours(geometry_data* geo, vec4 colour) {}