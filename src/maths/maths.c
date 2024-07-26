#include "maths.h"

Mat4 mat4_ident() {
  return (Mat4){ .data = { 1.0, 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1.0 } };
}