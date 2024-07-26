#include "maths.h"

#define c_static_inline

c_static_inline Vec3 vec3_create(f32 x, f32 y, f32 z) { return (Vec3){ x, y, z }; }
c_static_inline Vec3 vec3_add(Vec3 a, Vec3 b) { return (Vec3){ a.x + b.x, a.y + b.y, a.z + b.z }; }
c_static_inline Vec3 vec3_sub(Vec3 a, Vec3 b) { return (Vec3){ a.x - b.x, a.y - b.y, a.z - b.z }; }
c_static_inline Vec3 vec3_mult(Vec3 a, f32 s) { return (Vec3){ a.x * s, a.y * s, a.z * s }; }
c_static_inline Vec3 vec3_div(Vec3 a, f32 s) { return (Vec3){ a.x / s, a.y / s, a.z / s }; }

c_static_inline f32 vec3_len_squared(Vec3 a) { return (a.x * a.x) + (a.y * a.y) + (a.z * a.z); }
c_static_inline f32 vec3_len(Vec3 a) { return sqrtf(vec3_len_squared(a)); }
c_static_inline Vec3 vec3_negate(Vec3 a) { return (Vec3){ -a.x, -a.y, -a.z }; }
PUB c_static_inline Vec3 vec3_normalise(Vec3 a) {
  f32 length = vec3_len(a);
  return vec3_div(a, length);
}

c_static_inline f32 vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
c_static_inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
  return (
      Vec3){ .x = a.y * b.z - a.z * b.y, .y = a.z * b.x - a.x * b.z, .z = a.x * b.y - a.y * b.x };
}

Mat4 mat4_ident() {
  return (Mat4){ .data = { 1.0, 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1.0 } };
}