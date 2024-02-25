#include <glfw3.h>

#include "core.h"
#include "render.h"

int main() {
  core* core = core_bringup();

  // Main loop
  while (!glfwWindowShouldClose(core->renderer.window)) {
    input_update(&core->input);
    threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);

    // insert work here

    render_frame_end(&core->renderer);
  }

  return 0;
}