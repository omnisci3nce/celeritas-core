// The engine "core"

#include <celeritas.h>

NAMESPACED_LOGGER(core);

void core_bringup(const char* window_name, struct GLFWwindow* optional_window) {
  // INFO("Initiate Core bringup");
  INFO("Initiate Core bringup");

  INFO("Create GLFW window");
}
void core_shutdown() {}

bool app_should_exit() { return false; }
