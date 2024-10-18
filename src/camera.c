#include <celeritas.h>

mat4 camera_view_proj(camera camera, f32 lens_height, f32 lens_width, mat4* out_view, mat4* out_proj) {
  mat4 projection_matrix = mat4_perspective(camera.fov, lens_width / lens_height, 0.1, 1000.0);
  // TODO: store near/far on camera rather than hard-coding here.

  vec3 camera_direction = vec3_add(camera.position, camera.forwards);
  mat4 view_matrix = mat4_look_at(camera.position, camera_direction, camera.up);
  if (out_view)
    *out_view = view_matrix;
  if (out_proj)
    *out_proj = projection_matrix;

  return mat4_mult(view_matrix, projection_matrix);
}