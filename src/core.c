// The engine "core"

#include <celeritas.h>

NAMESPACED_LOGGER(core);

void Core_Bringup(const char* window_name, struct GLFWwindow* optional_window) {
  // INFO("Initiate Core bringup");
  INFO("Initiate Core bringup");

  INFO("Create GLFW window");
}
void Core_Shutdown() {}

bool AppShouldExit() { return true; }
