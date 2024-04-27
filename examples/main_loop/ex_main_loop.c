#include <glfw3.h>

#include "camera.h"
#include "core.h"
#include "maths.h"
#include "render.h"

int main() {
  core* core = core_bringup();

  camera camera = camera_create(vec3_create(0, 0, 20), VEC3_NEG_Z, VEC3_Y, deg_to_rad(45.0));

  // Main loop
  while (!glfwWindowShouldClose(core->renderer.window)) {
    input_update(&core->input);
    // threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);

    static f32 x = 0.0;
    x += 0.01;
    mat4 model = mat4_translation(vec3(x, 0, 0));

    gfx_backend_draw_frame(&core->renderer, &camera, model, NULL);

    // insert work here

    render_frame_end(&core->renderer);
    glfwSwapBuffers(core->renderer.window);
    glfwPollEvents();
  }

  return 0;
}
