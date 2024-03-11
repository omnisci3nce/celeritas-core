#include <glfw3.h>

#include "camera.h"
#include "core.h"
#include "maths.h"
#include "maths_types.h"
#include "render.h"
#include "render_types.h"

int main() {
  core* core = core_bringup();

  // --- Set up our scene

  // 1. load model from disk
  model_handle backpack_handle =
      model_load_obj(core, "assets/models/obj/backpack/backpack.obj", true);
  model* backpack = &core->models->data[backpack_handle.raw];
  // 2. upload vertex data to gpu
  model_upload_meshes(&core->renderer, backpack);
  // 3. create a camera
  vec3 camera_pos = vec3(3., 4., 10.);
  vec3 camera_front = vec3_normalise(vec3_negate(camera_pos));
  camera cam = camera_create(camera_pos, camera_front, VEC3_Y, deg_to_rad(45.0));

  // --- Enter Main loop
  while (!glfwWindowShouldClose(core->renderer.window)) {
    input_update(&core->input);
    threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);

    // Draw the cube
    transform cube_tf =
        transform_create(VEC3_ZERO, quat_ident(), 2.0);  // make the backpack a bit bigger
    draw_model(&core->renderer, &cam, backpack, cube_tf);

    render_frame_end(&core->renderer);
  }

  return 0;
}
