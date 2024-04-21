#include <assert.h>
#include <glfw3.h>

#include "../example_scene.h"
#include "animation.h"
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

typedef struct game_state {
  camera camera;
  vec3 camera_euler;
  bool first_mouse_update;  // so the camera doesnt lurch when you run the first
                            // process_camera_rotation
} game_state;

void update_camera_rotation(input_state* input, game_state* game, camera* cam);

int main() {
  double currentFrame = glfwGetTime();
  double lastFrame = currentFrame;
  double deltaTime;

  core* core = core_bringup();

  model_handle handle =
      model_load_gltf(core, "assets/models/gltf/SimpleSkin/glTF/SimpleSkin.gltf", false);

  model* simple_skin = &core->models->data[handle.raw];

  // Okay, right here we've loaded the model. let's assert some facts
  assert(simple_skin->animations->len == 1);
  assert(simple_skin->animations->data[0].rotation != NULL);
  assert(simple_skin->animations->data[0].translation == NULL);
  assert(simple_skin->animations->data[0].scale == NULL);

  mesh* m = &simple_skin->meshes->data[0];
  assert(m->is_skinned);
  assert(m->bones->len == 2);  // 1 root and 1 extra joint

  // assert(false);

  model_upload_meshes(&core->renderer, simple_skin);

  scene our_scene = make_default_scene();

  vec3 cam_pos = vec3_create(0, 5, -8);
  game_state game = {
    .camera = camera_create(cam_pos, vec3_negate(cam_pos), VEC3_Y, deg_to_rad(45.0)),
    .camera_euler = vec3_create(90, 0, 0),
    .first_mouse_update = true,
  };

  print_vec3(game.camera.front);

  // Main loop
  const f32 camera_lateral_speed = 0.2;
  const f32 camera_zoom_speed = 0.10;

  // animation
  animation_clip track = simple_skin->animations->data[0];
  f64 total_time = 0.0;

  while (!should_exit(core)) {
    input_update(&core->input);

    currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    total_time += deltaTime;
    // printf("delta time %f\n", deltaTime);
    f64 t = fmod(total_time, track.rotation->max);
    // INFO("Total time: %f", t);

    vec3 translation = VEC3_ZERO;
    if (key_is_pressed(KEYCODE_W)) {
      translation = vec3_mult(game.camera.front, camera_zoom_speed);
    } else if (key_is_pressed(KEYCODE_KEY_UP)) {
      translation = vec3_mult(game.camera.up, camera_lateral_speed);
    } else if (key_is_pressed(KEYCODE_KEY_DOWN)) {
      translation = vec3_mult(game.camera.up, -camera_lateral_speed);
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

    render_frame_begin(&core->renderer);

    // bone rotation
    quat rot = animation_sample(track.rotation, t).rotation;

    m->bones->data[1].transform_components.rotation = rot;

    // quat rot = quat_ident();
    transform tf = transform_create(VEC3_ZERO, quat_ident(), 1.0);

    draw_skinned_model(&core->renderer, &game.camera, simple_skin, tf, &our_scene);

    render_frame_end(&core->renderer);
  }

  INFO("Shutting down");
  model_destroy(simple_skin);

  core_shutdown(core);

  return 0;
}
