// Example demonstrating basic RAL usage by rendering a triangle

#include <celeritas.h>

int main() {
  core_bringup("Celeritas Example: Triangle", NULL);

  while (!app_should_exit()) {
    glfwPollEvents();
  }

  return 0;
}
