#include <glfw3.h>

#include "core.h"
#include "render.h"
#include "render_types.h"
#include "maths_types.h"
#include "maths.h"
#include "transform_hierarchy.h"

const vec3 pointlight_positions[4] = {
  { 0.7, 0.2, 2.0 },
  { 2.3, -3.3, -4.0 },
  { -4.0, 2.0, -12.0 },
  { 0.0, 0.0, -3.0 },
};
point_light point_lights[4];

int main() {
  core* core = core_bringup();

  // Set up scene
  vec3 camera_pos = vec3(3., 4., 10.);
  vec3 camera_front = vec3_normalise(vec3_negate(camera_pos));
  camera cam = camera_create(camera_pos, camera_front, VEC3_Y, deg_to_rad(45.0));

  model_handle cube_handle =
      model_load_obj(core, "assets/models/obj/cube/cube.obj", true);
  model* cube = &core->models->data[cube_handle.raw];
  model_upload_meshes(&core->renderer, cube);

  directional_light dir_light = { .direction = (vec3){ -0.2, -1.0, -0.3 },
                                  .ambient = (vec3){ 0.2, 0.2, 0.2 },
                                  .diffuse = (vec3){ 0.5, 0.5, 0.5 },
                                  .specular = (vec3){ 1.0, 1.0, 1.0 } };
  // point lights setup
  for (int i = 0; i < 4; i++) {
    point_lights[i].position = pointlight_positions[i];
    point_lights[i].ambient = (vec3){ 0.05, 0.05, 0.05 };
    point_lights[i].diffuse = (vec3){ 0.8, 0.8, 0.8 };
    point_lights[i].specular = (vec3){ 1.0, 1.0, 1.0 };
    point_lights[i].constant = 1.0;
    point_lights[i].linear = 0.09;
    point_lights[i].quadratic = 0.032;
  }

  scene our_scene = { .dir_light = dir_light, .n_point_lights = 4 };
  memcpy(&our_scene.point_lights, &point_lights, sizeof(point_light[4]));

  // Create transform hierarchy
  transform_hierarchy* transform_tree = transform_hierarchy_create();
  transform_node* root_node = transform_hierarchy_root_node(transform_tree);
  // Add nodes
  // -- 4 cubes
  transform cube1 = transform_create(vec3(0.0, -2.0, 0.0), quat_ident(), 0.8);
  transform cube2 = transform_create(vec3(0.0, 1.0, 0.0), quat_ident(), 1.0);
  transform cube3 = transform_create(vec3(0.0, 1.0, 0.0), quat_ident(), 1.0);
  transform cube4 = transform_create(vec3(0.0, 1.0, 0.0), quat_ident(), 1.0);
  transform_node* node1 = transform_hierarchy_add_node(root_node, cube_handle, cube1);
  transform_node* node2 = transform_hierarchy_add_node(node1, cube_handle, cube2);
  transform_node* node3 = transform_hierarchy_add_node(node2, cube_handle, cube3);
  transform_node* node4 = transform_hierarchy_add_node(node3, cube_handle, cube4);

  transform_hierarchy_debug_print(root_node, core);

  // Main loop
  while (!glfwWindowShouldClose(core->renderer.window)) {
    input_update(&core->input);
    threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);
    transform_hierarchy_propagate_transforms(transform_tree);

    node1->tf.position.x += 0.002;
    node1->tf.is_dirty = true;
    draw_model(&core->renderer, &cam, cube, &node1->world_matrix_tf, &our_scene);
    draw_model(&core->renderer, &cam, cube, &node2->world_matrix_tf, &our_scene);
    draw_model(&core->renderer, &cam, cube, &node3->world_matrix_tf, &our_scene);
    draw_model(&core->renderer, &cam, cube, &node4->world_matrix_tf, &our_scene);

    render_frame_end(&core->renderer);
  }

  transform_hierarchy_free(transform_tree);

  return 0;
}
