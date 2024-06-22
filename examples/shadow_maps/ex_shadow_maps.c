#include "celeritas.h"
#include "maths.h"
#include "maths_types.h"
#include "primitives.h"
#include "ral.h"
#include "ral_types.h"
#include "render.h"
#include "render_types.h"
#include "renderpasses.h"

extern core g_core;

// Scene / light setup
vec3 light_position = { -2, 4, -1 };
mesh s_scene[5];
transform s_transforms[5];

/*
  TODO:
- keyboard button to switch between main camera and light camera
*/

void draw_scene();

int main() {
  core_bringup();
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  vec3 camera_pos = vec3(2., 2., 2.);
  vec3 camera_front = vec3_normalise(vec3_negate(camera_pos));
  camera cam = camera_create(camera_pos, camera_front, VEC3_Y, deg_to_rad(45.0));

  ren_shadowmaps shadows = { .width = 1000, .height = 1000 };
  ren_shadowmaps_init(&shadows);

  // Set up the scene
  // We want:
  // 1. a ground plane
  // 2. lights
  // 3. some boxes
  for (int i = 0; i < 4; i++) {
    geometry_data geo = geo_create_cuboid(f32x3(2, 2, 2));
    s_scene[i] = mesh_create(&geo, true);
    s_transforms[i] = transform_create(vec3(4 * i, 0, 0), quat_ident(), 1.0);
  }
  geometry_data plane = geo_create_plane(f32x2(20, 20));
  s_scene[4] = mesh_create(&plane, true);

  geometry_data quad_geo = geo_create_plane(f32x2(2,2));
  mesh quad = mesh_create(&quad_geo, true);

  shader_data model_data = { .data = NULL, .shader_data_get_layout = &model_uniform_layout };
  shader_data lightspace_data = { .data = NULL,
                                  .shader_data_get_layout = &lightspace_uniform_layout };

  // Main loop
  while (!should_exit(&g_core)) {
    input_update(&g_core.input);

    if (!gpu_backend_begin_frame()) {
      continue;
    }
    gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();

    gpu_cmd_encoder_begin_render(enc, shadows.rpass);

    // calculations
    f32 near_plane = 1.0, far_plane = 7.5;
    mat4 light_projection = mat4_orthographic(-10.0, 10.0, -10.0, 10.0, near_plane, far_plane);
    mat4 light_view = mat4_look_at(light_position, VEC3_ZERO, VEC3_Y);
    mat4 light_space_matrix = mat4_mult(light_view, light_projection);
    lightspace_tf_uniform lsu = { .lightSpaceMatrix = light_space_matrix };

    encode_bind_pipeline(enc, PIPELINE_GRAPHICS, shadows.static_pipeline);

    lightspace_data.data = &lsu;
    encode_bind_shader_data(enc, 0, &lightspace_data);

    draw_scene();

    gpu_cmd_encoder_end_render(enc);

    gpu_cmd_encoder_begin_render(enc, shadows.debug_quad->renderpass);
    encode_bind_pipeline(enc, PIPELINE_GRAPHICS, shadows.debug_quad);
    debug_quad_uniform dqu = {.depthMap = shadows.depth_tex};
    shader_data debug_quad_data = { .data = &dqu, .shader_data_get_layout = debug_quad_layout};
    encode_bind_shader_data(enc, 0, &debug_quad_data);
    encode_set_vertex_buffer(enc, quad.vertex_buffer);
    encode_set_index_buffer(enc, quad.index_buffer);
    encode_draw_indexed(enc, quad.geometry->indices->len);

    gpu_cmd_encoder_end_render(enc);

    // gpu_cmd_encoder_begin_render(enc, static_opaque_rpass);

    // gpu_cmd_encoder_end_render(enc);

    gpu_backend_end_frame();
  }

  renderer_shutdown(&g_core.renderer);

  return 0;
}

void draw_scene() {
  gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
  for (int i = 0; i < 5; i++) {
    model_uniform mu = { .model = transform_to_mat(&s_transforms[i]) };
    shader_data model_data = { .data = &mu, .shader_data_get_layout = model_uniform_layout };
    encode_bind_shader_data(enc, 0, &model_data);
    encode_set_vertex_buffer(enc, s_scene[i].vertex_buffer);
    encode_set_index_buffer(enc, s_scene[i].index_buffer);
    encode_draw_indexed(enc, s_scene[i].geometry->indices->len);
  }
}
