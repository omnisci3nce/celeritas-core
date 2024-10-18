#include <celeritas.h>

vec3 vec3_create(f32 x, f32 y, f32 z) { return (vec3){ x, y, z }; }

vec3 vec3_add(vec3 u, vec3 v) {
  return (vec3){ .x = u.x + v.x, .y = u.y + v.y, .z = u.z + v.z };
}

vec4 vec4_create(f32 x, f32 y, f32 z, f32 w) { return (vec4){ x, y, z, w }; }

mat4 mat4_ident() { return (mat4){ .data = { 1.0, 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1.0 } }; }

mat4 mat4_mult(mat4 lhs, mat4 rhs) {
  mat4 out_matrix = mat4_ident();

  const f32* m1_ptr = lhs.data;
  const f32* m2_ptr = rhs.data;
  f32* dst_ptr = out_matrix.data;

  for (i32 i = 0; i < 4; ++i) {
    for (i32 j = 0; j < 4; ++j) {
      *dst_ptr = m1_ptr[0] * m2_ptr[0 + j] + m1_ptr[1] * m2_ptr[4 + j] + m1_ptr[2] * m2_ptr[8 + j] +
                 m1_ptr[3] * m2_ptr[12 + j];
      dst_ptr++;
    }
    m1_ptr += 4;
  }

  return out_matrix;
}

mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near_z, f32 far_z) {
  f32 half_tan_fov = tanf(fov_radians * 0.5f);
  mat4 out_matrix = { .data = { 0 } };
  out_matrix.data[0] = 1.0f / (aspect_ratio * half_tan_fov);
  out_matrix.data[5] = 1.0f / half_tan_fov;
  out_matrix.data[10] = -((far_z + near_z) / (far_z - near_z));
  out_matrix.data[11] = -1.0f;
  out_matrix.data[14] = -((2.0f * far_z * near_z) / (far_z - near_z));
  return out_matrix;
}

mat4 mat4_look_at(vec3 position, vec3 target, vec3 up) {
  // TODO
}