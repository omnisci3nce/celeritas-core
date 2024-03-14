#include "camera.h"

#include "maths.h"

camera camera_create(vec3 pos, vec3 front, vec3 up, f32 fov) {
  camera c = { .position = pos, .front = front, .up = up, .fov = fov };
  return c;
}

void camera_view_projection(camera *c, f32 screen_height, f32 screen_width, mat4 *out_view,
                            mat4 *out_proj) {
  mat4 proj = mat4_perspective(c->fov, screen_width / screen_height, 0.1, 100.0);
  vec3 camera_direction = vec3_add(c->position, c->front);
  mat4 view = mat4_look_at(c->position, camera_direction, c->up);
  *out_view = view;
  *out_proj = proj;
}