#include <glfw3.h>

#include "buf.h"
#include "camera.h"
#include "core.h"
#include "file.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "mem.h"
#include "primitives.h"
#include "ral.h"
#include "ral_types.h"
#include "render.h"
#include "render_types.h"

extern core g_core;

int main() {
  core_bringup();
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  vec3 camera_pos = vec3(2., 2., 2.);
  vec3 camera_front = vec3_normalise(vec3_negate(camera_pos));
  camera cam = camera_create(camera_pos, camera_front, VEC3_Y, deg_to_rad(45.0));

  // Geometry
  // geo_create_cuboid(f32x3(1, 1, 1));
  geometry_data sphere_data = geo_create_uvsphere(2.0, 8, 8);
  mesh sphere = mesh_create(&sphere_data, false);

  // Texture
  texture_data tex_data = texture_data_load("assets/textures/texture.jpg", false);
  texture_handle texture = texture_data_upload(tex_data, true);

  static f32 theta = 0.0;

  // Main loop
  while (!should_exit(&g_core)) {
    input_update(&g_core.input);

    render_frame_begin(&g_core.renderer);

    mat4 model = mat4_ident();
    draw_mesh(&sphere, &model, &cam);

    render_frame_end(&g_core.renderer);
  }

  renderer_shutdown(&g_core.renderer);

  return 0;
}
