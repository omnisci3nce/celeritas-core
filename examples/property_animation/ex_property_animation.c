#include <glfw3.h>

#include "camera.h"
#include "core.h"
#include "input.h"
#include "keys.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "primitives.h"
#include "render.h"
#include "render_backend.h"
#include "render_types.h"

const vec3 pointlight_positions[4] = {
  { 0.7, 0.2, 2.0 },
  { 2.3, -3.3, -4.0 },
  { -4.0, 2.0, -12.0 },
  { 0.0, 0.0, -3.0 },
};
point_light point_lights[4];

typedef struct game_state {
  camera camera;
  vec3 camera_euler;
  bool first_mouse_update;  // so the camera doesnt lurch when you run the first
                            // process_camera_rotation
} game_state;

void update_camera_rotation(input_state* input, game_state* game, camera* cam);

int main() {
  core* core = core_bringup();

  model_handle animated_cube_handle =
      model_load_gltf(core, "assets/models/gltf/AnimatedCube/glTF/AnimatedCube.gltf", false);
  model* cube = &core->models->data[animated_cube_handle.raw];
  model_upload_meshes(&core->renderer, cube);

  directional_light dir_light = { .direction = (vec3){ -0.2, -1.0, -0.3 },
                                  .ambient = (vec3){ 0.2, 0.2, 0.2 },
                                  .diffuse = (vec3){ 0.5, 0.5, 0.5 },
                                  .specular = (vec3){ 1.0, 1.0, 1.0 } };

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

  vec3 cam_pos = vec3_create(5, 5, 5);
  game_state game = {
    .camera = camera_create(cam_pos, vec3_negate(cam_pos), VEC3_Y, deg_to_rad(45.0)),
    .camera_euler = vec3_create(90, 0, 0),
    .first_mouse_update = true,
  };

  print_vec3(game.camera.front);

  // Main loop
  const f32 camera_lateral_speed = 0.2;
  const f32 camera_zoom_speed = 0.15;

  while (!should_exit(core)) {
    input_update(&core->input);

    vec3 translation = VEC3_ZERO;
    if (key_is_pressed(KEYCODE_W) || key_is_pressed(KEYCODE_KEY_UP)) {
      translation = vec3_mult(game.camera.front, camera_zoom_speed);
    } else if (key_is_pressed(KEYCODE_S) || key_is_pressed(KEYCODE_KEY_DOWN)) {
      translation = vec3_mult(game.camera.front, -camera_zoom_speed);
    } else if (key_is_pressed(KEYCODE_A) || key_is_pressed(KEYCODE_KEY_LEFT)) {
      vec3 lateral = vec3_normalise(vec3_cross(game.camera.front, game.camera.up));
      translation = vec3_mult(lateral, -camera_lateral_speed);
    } else if (key_is_pressed(KEYCODE_D) || key_is_pressed(KEYCODE_KEY_RIGHT)) {
      vec3 lateral = vec3_normalise(vec3_cross(game.camera.front, game.camera.up));
      translation = vec3_mult(lateral, camera_lateral_speed);
    }
    game.camera.position = vec3_add(game.camera.position, translation);

    // UNUSED: threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);

    mat4 model = mat4_translation(VEC3_ZERO);
    transform tf = transform_create(VEC3_ZERO, quat_ident(), 1.0);

    draw_model(&core->renderer, &game.camera, cube, tf, &our_scene);

    // gfx_backend_draw_frame(&core->renderer, &game.camera, model, NULL);

    render_frame_end(&core->renderer);
  }

  INFO("Shutting down");
  model_destroy(cube);

  core_shutdown(core);

  return 0;
}
