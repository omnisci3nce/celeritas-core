#include "render.h"

#include <glfw3.h>

#include "log.h"
#include "render_backend.h"

bool renderer_init(renderer* ren) {
  INFO("Renderer init");

  // NOTE: all platforms use GLFW at the moment but thats subject to change
  glfwInit();

  // glfw window creation
  GLFWwindow* window = glfwCreateWindow(ren->config.scr_width, ren->config.scr_height,
                                        ren->config.window_name, NULL, NULL);
  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return false;
  }
  ren->window = window;

  glfwMakeContextCurrent(ren->window);

  if (!gfx_backend_init(ren)) {
    FATAL("Couldnt load graphics api backend");
    return false;
  }

  return true;
}

void render_frame_begin(renderer* ren) {
  vec3 color = ren->config.clear_colour;
  clear_screen(color);
}
void render_frame_end(renderer* ren) {
  // present frame
  glfwSwapBuffers(ren->window);
  glfwPollEvents();
}