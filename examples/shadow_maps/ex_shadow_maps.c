#include "celeritas.h"
#include "maths_types.h"
#include "primitives.h"
#include "ral.h"
#include "render.h"
#include "render_types.h"
#include "renderpasses.h"

extern core g_core;

// Scene / light setup
const vec3 pointlight_positions[4] = {
  { -10.0, 10.0, 10.0 },
  { 10.0, 10.0, 10.0 },
  { -10.0, -10.0, 10.0 },
  { 10.0, -10.0, 10.0 },
};
point_light point_lights[4];

int main() {
  core_bringup();
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  vec3 camera_pos = vec3(2., 2., 2.);
  vec3 camera_front = vec3_normalise(vec3_negate(camera_pos));
  camera cam = camera_create(camera_pos, camera_front, VEC3_Y, deg_to_rad(45.0));

  ren_shadowmaps shadows = { .width = 1000, .height = 1000 };
  // ren_shadowmaps_init(&shadows);

  // Meshes
  mesh cubes[4];
  for (int i = 0; i < 4; i++) {
    geometry_data geo = geo_create_cuboid(f32x3(2,2,2));
    cubes[i] = mesh_create(&geo, true);
  }

  // Main loop
  while (!should_exit(&g_core)) {
    input_update(&g_core.input);

    if (!gpu_backend_begin_frame()) {
      continue;
    }
    gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();

    gpu_cmd_encoder_begin_render(enc, shadows.rpass);
    encode_bind_pipeline(enc, PIPELINE_GRAPHICS, shadows.static_pipeline);
    for (int i = 0; i < 4; i++) {
      encode_set_vertex_buffer(enc, cubes[i].vertex_buffer);
      encode_set_index_buffer(enc, cubes[i].index_buffer);
      encode_draw_indexed(enc, cubes[i].geometry->indices->len);
    }

    gpu_cmd_encoder_end_render(enc);

    // gpu_cmd_encoder_begin_render(enc, static_opaque_rpass);

    gpu_cmd_encoder_end_render(enc);
    /*
    Shadows

    render scene into texture

    begin_renderpass()
    bind_pipeline()
    upload shader data
    for each object:
      - set buffers
      - draw call

    end_renderpass()
    */
    
    gpu_backend_end_frame();
  }

  renderer_shutdown(&g_core.renderer);

  return 0;
}
