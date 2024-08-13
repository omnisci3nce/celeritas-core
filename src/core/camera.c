#include "camera.h"

#include "input.h"
#include "keys.h"
#include "maths.h"

#define CAMERA_SPEED 0.2
#define CAMERA_SENSITIVITY 0.5

Camera Camera_Create(Vec3 pos, Vec3 front, Vec3 up, f32 fov) {
  Camera c = { .position = pos, .front = front, .up = up, .fov = fov };
  return c;
}

Mat4 Camera_ViewProj(Camera* c, f32 lens_height, f32 lens_width, Mat4* out_view, Mat4* out_proj) {
  Mat4 proj = mat4_perspective(c->fov, lens_width / lens_height, 0.1, 1000.0);
  Vec3 camera_direction = vec3_add(c->position, c->front);
  Mat4 view = mat4_look_at(c->position, camera_direction, c->up);
  if (out_view) {
    *out_view = view;
  }
  if (out_proj) {
    *out_proj = proj;
  }
  return mat4_mult(view, proj);
}

void FlyCamera_Update(Camera* camera) {
  static f32 yaw = 0.0;
  static f32 pitch = 0.0;

  // Keyboard
  f32 speed = CAMERA_SPEED;
  Vec3 horizontal = vec3_cross(camera->front, camera->up);
  if (key_is_pressed(KEYCODE_A) || key_is_pressed(KEYCODE_KEY_LEFT)) {
    Vec3 displacement = vec3_mult(horizontal, -speed);
    camera->position = vec3_add(camera->position, displacement);
  }
  if (key_is_pressed(KEYCODE_D) || key_is_pressed(KEYCODE_KEY_RIGHT)) {
    Vec3 displacement = vec3_mult(horizontal, speed);
    camera->position = vec3_add(camera->position, displacement);
  }
  if (key_is_pressed(KEYCODE_W) || key_is_pressed(KEYCODE_KEY_UP)) {
    Vec3 displacement = vec3_mult(camera->front, speed);
    camera->position = vec3_add(camera->position, displacement);
  }
  if (key_is_pressed(KEYCODE_S) || key_is_pressed(KEYCODE_KEY_DOWN)) {
    Vec3 displacement = vec3_mult(camera->front, -speed);
    camera->position = vec3_add(camera->position, displacement);
  }
  if (key_is_pressed(KEYCODE_Q)) {
    Vec3 displacement = vec3_mult(camera->up, speed);
    camera->position = vec3_add(camera->position, displacement);
  }
  if (key_is_pressed(KEYCODE_E)) {
    Vec3 displacement = vec3_mult(camera->up, -speed);
    camera->position = vec3_add(camera->position, displacement);
  }

  // Mouse
  if (MouseBtn_Held(MOUSEBTN_LEFT)) {
    mouse_state mouse = Input_GetMouseState();
    // printf("Delta x: %d Delta y %d\n",mouse.x_delta, mouse.y_delta );

    f32 x_offset = mouse.x_delta;
    f32 y_offset = -mouse.y_delta;

    f32 sensitivity = CAMERA_SENSITIVITY;  // change this value to your liking
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw += x_offset;
    pitch += y_offset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    Vec3 front;
    front.x = cos(deg_to_rad(yaw) * cos(deg_to_rad(pitch)));
    front.y = sin(deg_to_rad(pitch));
    front.z = sin(deg_to_rad(yaw)) * cos(deg_to_rad(pitch));
    front = vec3_normalise(front);
    camera->front.x = front.x;
    camera->front.y = front.y;
    camera->front.z = front.z;
  }

  // TODO: Right mouse => pan in screen space
}
