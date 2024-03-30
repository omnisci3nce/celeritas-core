#include <glfw3.h>

#include "camera.h"
#include "core.h"
#include "input.h"
#include "keys.h"
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
  core* core = core_bringup();

  vec3 cam_pos = vec3_create(-15, 20.0, 13);
  game_state game = {
    .camera = camera_create(cam_pos, vec3_negate(cam_pos), VEC3_Y, deg_to_rad(45.0)),
    .camera_euler = vec3_create(90, 0, 0),
    .first_mouse_update = true,
  };

  // model_handle cube_handle = prim_cube_new(core);

  printf("Starting look direction: ");
  print_vec3(game.camera.front);

  // Main loop
  const f32 camera_speed = 0.4;

  while (!should_exit(core)) {
    input_update(&core->input);

    vec3 translation = VEC3_ZERO;
    if (key_is_pressed(KEYCODE_W) || key_is_pressed(KEYCODE_KEY_UP)) {
      printf("Move Forwards\n");
      translation = vec3_mult(game.camera.front, camera_speed);
    } else if (key_is_pressed(KEYCODE_S) || key_is_pressed(KEYCODE_KEY_DOWN)) {
      printf("Move Backwards\n");
      translation = vec3_mult(game.camera.front, -camera_speed);
    } else if (key_is_pressed(KEYCODE_A) || key_is_pressed(KEYCODE_KEY_LEFT)) {
      printf("Move Left\n");
      vec3 lateral = vec3_normalise(vec3_cross(game.camera.front, game.camera.up));
      translation = vec3_mult(lateral, -camera_speed);
    } else if (key_is_pressed(KEYCODE_D) || key_is_pressed(KEYCODE_KEY_RIGHT)) {
      printf("Move Right\n");
      vec3 lateral = vec3_normalise(vec3_cross(game.camera.front, game.camera.up));
      translation = vec3_mult(lateral, camera_speed);
    }
    game.camera.position = vec3_add(game.camera.position, translation);
    // update_camera_rotation(&core->input, &game, &game.camera);

    // UNUSED: threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);

    // model cube = core->models->data[cube_handle.raw];
    mat4 model = mat4_translation(VEC3_ZERO);

    gfx_backend_draw_frame(&core->renderer, &game.camera, model);

    render_frame_end(&core->renderer);
  }

  core_shutdown(core);

  return 0;
}

void update_camera_rotation(input_state* input, game_state* game, camera* cam) {
  float xoffset = -input->mouse.x_delta;  // xpos - lastX;
  float yoffset = -input->mouse.y_delta;  // reversed since y-coordinates go from bottom to top
  if (game->first_mouse_update) {
    xoffset = 0.0;
    yoffset = 0.0;
    game->first_mouse_update = false;
  }

  float sensitivity = 0.1f;  // change this value to your liking
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  // x = yaw
  game->camera_euler.x += xoffset;
  // y = pitch
  game->camera_euler.y += yoffset;
  // we dont update roll

  f32 yaw = game->camera_euler.x;
  f32 pitch = game->camera_euler.y;

  // make sure that when pitch is out of bounds, screen doesn't get flipped
  if (game->camera_euler.y > 89.0f) game->camera_euler.y = 89.0f;
  if (game->camera_euler.y < -89.0f) game->camera_euler.y = -89.0f;

  vec3 front = cam->front;
  front.x = cos(deg_to_rad(yaw) * cos(deg_to_rad(pitch)));
  front.y = sin(deg_to_rad(pitch));
  front.z = sin(deg_to_rad(yaw)) * cos(deg_to_rad(pitch));

  front = vec3_normalise(front);
  // save it back
  cam->front.x = front.x;
  cam->front.y = front.y;
  // roll is static

  print_vec3(cam->front);
}
