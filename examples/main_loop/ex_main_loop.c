#include <glfw3.h>

#include "core.h"
#include "maths.h"
#include "render.h"
#include "render_backend.h"

int main() {
  core* core = core_bringup();

  // Main loop
  while (!glfwWindowShouldClose(core->renderer.window)) {
    input_update(&core->input);
    threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);

    gfx_backend_draw_frame(&core->renderer);

    // insert work here

    render_frame_end(&core->renderer);
  }

  return 0;
}
