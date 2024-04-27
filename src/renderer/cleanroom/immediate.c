#include "immediate.h"
#include "maths.h"
#include "primitives.h"
#include "render.h"
#include "types.h"

void imm_draw_sphere(vec3 pos, f32 radius, vec4 colour) {
  // Create the vertices
  geometry_data geometry = geo_create_uvsphere(radius, 16, 16);
  geo_set_vertex_colours(&geometry, colour);

  // Upload to GPU
  mat4 model = mat4_translation(pos);

  // Set pipeline

  // Draw
}