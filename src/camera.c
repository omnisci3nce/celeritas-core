#include "camera.h"

#include "maths.h"

void camera_view_projection(camera *c, f32 screen_height, f32 screen_width, mat4 *out_view_proj) {
  mat4 proj = mat4_perspective(c->fov * 3.14 / 180.0, screen_width / screen_height, 0.1, 100.0);
  vec3 camera_direction = vec3_add(c->position, c->front);
  mat4 view = mat4_look_at(c->position, camera_direction, c->up);
  mat4 out_mat = mat4_mult(view, proj);
  *out_view_proj = out_mat;
}