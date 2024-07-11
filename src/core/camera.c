#include "camera.h"

#include "maths.h"

Camera Camera_Create(vec3 pos, vec3 front, vec3 up, f32 fov) {
  Camera c = { .position = pos, .front = front, .up = up, .fov = fov };
  return c;
}

mat4 Camera_ViewProj(Camera *c, f32 lens_height, f32 lens_width, mat4 *out_view,
                            mat4 *out_proj) {
  mat4 proj = mat4_perspective(c->fov, lens_width / lens_height, 0.1, 100.0);
  vec3 camera_direction = vec3_add(c->position, c->front);
  mat4 view = mat4_look_at(c->position, camera_direction, c->up);
  if (out_view) {
    *out_view = view;
  }
  if (out_proj) {
    *out_proj = proj;
  }
  return mat4_mult(view, proj);
}
