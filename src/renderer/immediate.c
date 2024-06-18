#include "immediate.h"
#include "glad/glad.h"
#include "maths.h"
#include "primitives.h"
#include "ral_types.h"
#include "render.h"
#include "render_types.h"

typedef struct immdraw_system {
  // primitive meshes (get reused for each draw call)
  mesh plane;
  mesh cube;
  mesh sphere;
  // command lists

} immdraw_system;

bool immdraw_system_init(immdraw_system* state) {
  geometry_data plane_geometry = geo_create_plane(f32x2(1, 1));
  state->plane = mesh_create(&plane_geometry, true);

  geometry_data cube_geometry = geo_create_cuboid(f32x3(1, 1, 1));
  state->cube = mesh_create(&cube_geometry, true);

  geometry_data sphere_geometry = geo_create_uvsphere(1.0, 48, 48);
  state->sphere = mesh_create(&sphere_geometry, true);

  return true;
}

void immdraw_plane(vec3 pos, quat rotation, f32 u_scale, f32 v_scale, vec4 colour) {}

void immdraw_system_render(immdraw_system* state) {}

// void imm_draw_sphere(vec3 pos, f32 radius, vec4 colour) {
//   // Create the vertices
//   geometry_data geometry = geo_create_uvsphere(radius, 16, 16);
//   geo_set_vertex_colours(&geometry, colour);

//   // Upload to GPU
//   mat4 model = mat4_translation(pos);

//   // Set pipeline

//   // Draw
// }