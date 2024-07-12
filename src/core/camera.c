#include "camera.h"

#include "maths.h"

Camera Camera_Create(Vec3 pos, Vec3 front, Vec3 up, f32 fov) {
  Camera c = { .position = pos, .front = front, .up = up, .fov = fov };
  return c;
}

Mat4 Camera_ViewProj(Camera *c, f32 lens_height, f32 lens_width, Mat4 *out_view, Mat4 *out_proj) {
  Mat4 proj = mat4_perspective(c->fov, lens_width / lens_height, 0.1, 100.0);
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
