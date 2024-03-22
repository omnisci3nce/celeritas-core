#include <glfw3.h>

#include "core.h"
#include "mem.h"
#include "render.h"
#include "maths.h"

int main() {
  core* core = core_bringup();

  vec3 light_position = vec3_create(-2.0, 4.0, -1.0);

  model_handle cube_handle = model_load_obj(core, "assets/models/obj/cube/cube.obj", true);
  model* cube = &core->models->data[cube_handle.raw];
  model_upload_meshes(&core->renderer, cube);

  transform_hierarchy* transform_tree = transform_hierarchy_create();
  transform_node* root_node = transform_hierarchy_root_node(transform_tree);
  transform cube1 = transform_create(vec3(0.0, -2.0, 0.0), quat_ident(), 0.8);

directional_light dir_light = { .direction = (vec3){ -0.2, -1.0, -0.3 },
                                  .ambient = (vec3){ 0.2, 0.2, 0.2 },
                                  .diffuse = (vec3){ 0.5, 0.5, 0.5 },
                                  .specular = (vec3){ 1.0, 1.0, 1.0 } };
  scene our_scene = { .dir_light = dir_light, .n_point_lights = 0 };

  char* frame_allocator_storage = malloc(1024 * 1024 * 64);
  arena frame_arena = arena_create(frame_allocator_storage, 1024 * 1024 * 64);

  // Main loop
  while (!glfwWindowShouldClose(core->renderer.window)) {
    input_update(&core->input);
    // threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);
    transform_hierarchy_propagate_transforms(transform_tree);
    draw_shadows(&core->renderer, &frame_arena, core->models, light_position, transform_tree, &our_scene);

    // insert work here

    render_frame_end(&core->renderer);
    arena_free_all(&frame_arena);
  }

  return 0;
}
