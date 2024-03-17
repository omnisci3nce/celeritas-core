#include <glfw3.h>

#include "core.h"
#include "render.h"
#include "render_types.h"
#include "maths_types.h"
#include "maths.h"
#include "transform_hierarchy.h"

int main() {
  core* core = core_bringup();

  // Set up scene
  vec3 camera_pos = vec3(3., 4., 10.);
  vec3 camera_front = vec3_normalise(vec3_negate(camera_pos));
  camera cam = camera_create(camera_pos, camera_front, VEC3_Y, deg_to_rad(45.0));

  model_handle cube_handle =
      model_load_obj(core, "assets/models/obj/cube/cube.obj", true);
  model* cube = &core->models->data[cube_handle.raw];
  // 2. upload vertex data to gpu
  model_upload_meshes(&core->renderer, cube);

  // Create transform hierarchy
  transform_hierarchy* transform_tree = transform_hierarchy_create();
  transform_node* root_node = transform_hierarchy_root_node(transform_tree);

  // Add nodes
  // -- 4 cubes
  transform cube1 = transform_create(vec3(-2.0, -2.0, -2.0), quat_ident(), 2.0);
  transform cube2 = transform_create(vec3(2.0, 2.0, 2.0), quat_ident(), 2.0);
  transform_hierarchy_add_node(root_node, cube_handle, cube1);
  transform_hierarchy_add_node(root_node, cube_handle, cube2);


  // Main loop
  while (!glfwWindowShouldClose(core->renderer.window)) {
    input_update(&core->input);
    threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);

    // insert work here

    render_frame_end(&core->renderer);
  }

  transform_hierarchy_free(transform_tree);

  return 0;
}
