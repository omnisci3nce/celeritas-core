#include <glfw3.h>
#include <string.h>

#include "camera.h"
#include "core.h"
#include "maths.h"
#include "maths_types.h"
#include "render.h"
#include "render_types.h"

const vec3 pointlight_positions[4] = {
    {0.7, 0.2, 2.0},
    {2.3, -3.3, -4.0},
    {-4.0, 2.0, -12.0},
    {0.0, 0.0, -3.0},
};
point_light point_lights[4];

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
  // 4. create lights

  // directional (sun) light setup
  directional_light dir_light = {.direction = (vec3){-0.2, -1.0, -0.3},
                                 .ambient = (vec3){0.2, 0.2, 0.2},
                                 .diffuse = (vec3){0.5, 0.5, 0.5},
                                 .specular = (vec3){1.0, 1.0, 1.0}};
  // point lights setup
  for (int i = 0; i < 4; i++) {
    point_lights[i].position = pointlight_positions[i];
    point_lights[i].ambient = (vec3){0.05, 0.05, 0.05};
    point_lights[i].diffuse = (vec3){0.8, 0.8, 0.8};
    point_lights[i].specular = (vec3){1.0, 1.0, 1.0};
    point_lights[i].constant = 1.0;
    point_lights[i].linear = 0.09;
    point_lights[i].quadratic = 0.032;
  }

  scene our_scene = {
    .dir_light = dir_light,
    .n_point_lights = 4
  };
  memcpy(&our_scene.point_lights, &point_lights, sizeof(point_light[4]));


  // --- Enter Main loop
  while (!glfwWindowShouldClose(core->renderer.window)) {
    input_update(&core->input);
    threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);

    // Draw the backpack
    transform model_tf =
        transform_create(VEC3_ZERO, quat_ident(), 2.0);  // make the backpack a bit bigger
    draw_model(&core->renderer, &cam, backpack, model_tf, &our_scene);

    render_frame_end(&core->renderer);
  }

  return 0;
}
