// The engine "core"

#include <celeritas.h>
#include <stdlib.h>

NAMESPACED_LOGGER(core);

core g_core = {0};

#ifdef GPU_METAL
static const char* gapi = "Metal";
#else
static const char* gapi = "Vulkan";
#endif

// forward declares
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);

void core_bringup(const char* window_name, struct GLFWwindow* optional_window) {
  INFO("Initiate Core bringup");

  INFO("Create GLFW window");
  glfwInit();

  char* full_window_name = malloc(sizeof(char) * 100);
  int _offset = sprintf(full_window_name, "%s (%s)", window_name, gapi);

  GLFWwindow* glfw_window = glfwCreateWindow(800, 600, full_window_name, NULL, NULL);
  g_core.window = glfw_window;

  // This may move into a renderer struct
  ral_backend_init(window_name, glfw_window);

  glfwSetKeyCallback(glfw_window, key_callback);
}
void core_shutdown() {
  ral_backend_shutdown();
  glfwTerminate();
}

bool app_should_exit() {
  return glfwWindowShouldClose(g_core.window) || g_core.should_exit;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
        g_core.should_exit = true;
    }
}