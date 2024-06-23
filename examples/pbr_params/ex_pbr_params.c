#include <glfw3.h>
#include <stdbool.h>

#include "builtin_materials.h"
#include "camera.h"
#include "colours.h"
#include "core.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "primitives.h"
#include "ral.h"
#include "render.h"
#include "render_types.h"
#include "str.h"

extern core g_core;

const vec3 pointlight_positions[4] = {
  { 10.0 / 1., 10.0 / 1., 10.0 },
  { -10.0 / 1., 10.0 / 1., 10.0 },
  { 10.0 / 1., -10.0 / 1., 10.0 },
  { -10.0 / 1., -10.0 / 1., 10.0 },
};
pbr_point_light point_lights[4];

void encode_draw_mesh(mesh* m);

const int num_rows = 7, num_cols = 7;
const float spacing = 2.5;

pbr_params_material_uniforms pbr_params;

int main() {
  core_bringup();

  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  for (int i = 0; i < 4; i++) {
    point_lights[i].pos = pointlight_positions[i];
    point_lights[i].color = vec3(300.0, 300.0, 300.0);
  }

  vec3 camera_pos = vec3(-1., -1., 26.);
  camera cam = camera_create(camera_pos, VEC3_NEG_Z, VEC3_Y, deg_to_rad(45.0));

  // PBR Material data
  pbr_params.albedo = (vec3){ 0.5, 0.0, 0.0 };
  pbr_params.ao = 1.0;
  shader_data pbr_uniforms = { .data = NULL, .shader_data_get_layout = &pbr_params_shader_layout };

  // Make the pipeline
  gpu_renderpass_desc pass_description = { .default_framebuffer = true, .color_target = true };
  gpu_renderpass* renderpass = gpu_renderpass_create(&pass_description);
  // FIXME: Make this work on other API backends
  str8 vert_path = str8lit("assets/shaders/pbr_params.vert");
  str8 frag_path = str8lit("assets/shaders/pbr_params.frag");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  struct graphics_pipeline_desc pipeline_description = {
    .debug_name = "Basic Pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = { pbr_uniforms },
    .data_layouts_count = 1,
    .vs = { .debug_name = "PBR (params) Vertex Shader",
            .filepath = vert_path,
            .code = vertex_shader.contents,
            .is_spirv = false },
    .fs = { .debug_name = "PBR (params) Fragment Shader",
            .filepath = frag_path,
            .code = fragment_shader.contents,
            .is_spirv = false },
    .renderpass = renderpass,
    .wireframe = false,
    .depth_test = false
  };
  gpu_pipeline* pbr_pipeline = gpu_graphics_pipeline_create(pipeline_description);

  // Geometry
  geometry_data sphere_data = geo_create_uvsphere(1.0, 64, 64);
  mesh sphere = mesh_create(&sphere_data, false);

  geometry_data cube_data = geo_create_cuboid(f32x3(1, 1, 1));
  mesh cube = mesh_create(&cube_data, false);

  pbr_params_bindgroup pbr_bind_data;

  while (!should_exit()) {
    input_update(&g_core.input);

    if (!gpu_backend_begin_frame()) {
      continue;
    }
    gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
    // Begin recording
    gpu_cmd_encoder_begin(*enc);
    gpu_cmd_encoder_begin_render(enc, renderpass);
    encode_bind_pipeline(enc, PIPELINE_GRAPHICS, pbr_pipeline);
    encode_set_default_settings(enc);

    mat4 model_affine = mat4_ident();
    mat4 view, proj;
    camera_view_projection(&cam, 1000, 1000, &view, &proj);

    // Feed shader data
    pbr_bind_data.mvp_matrices =
        (mvp_matrix_uniforms){ .model = model_affine, .view = view, .projection = proj };
    pbr_bind_data.material = pbr_params;
    pbr_bind_data.lights = (pbr_params_light_uniforms){
      .viewPos = vec4(cam.position.x, cam.position.y, cam.position.z, 1.0),
      .pointLights = { point_lights[0], point_lights[1], point_lights[2], point_lights[3] }
    };
    pbr_uniforms.data = &pbr_bind_data;

    // Record draw calls
    for (u32 row = 0; row < num_rows; row++) {
      f32 metallic = (float)row / (float)num_rows;
      for (u32 col = 0; col < num_cols; col++) {
        f32 roughness = (float)col / (float)num_cols;
        if (roughness == 0.0) {
          roughness += 0.05;
        };
        if (roughness == 1.0) {
          roughness -= 0.05;
        };

        pbr_bind_data.material.metallic = metallic;
        pbr_bind_data.material.roughness = roughness;

        f32 x = (col - ((f32)num_cols / 2.)) * spacing;
        f32 y = (row - ((f32)num_rows / 2.)) * spacing;
        mat4 model = mat4_translation(vec3(x, y, 0.0f));
        pbr_bind_data.mvp_matrices.model = model;
        encode_bind_shader_data(enc, 0, &pbr_uniforms);
        mesh object = sphere;
        encode_draw_mesh(&object);
      }
    }

    // End recording
    gpu_cmd_encoder_end_render(enc);

    gpu_cmd_buffer buf = gpu_cmd_encoder_finish(enc);
    gpu_queue_submit(&buf);
    // Submit
    gpu_backend_end_frame();
  }

  renderer_shutdown(&g_core.renderer);

  return 0;
}

void encode_draw_mesh(mesh* m) {
  gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
  encode_set_vertex_buffer(enc, m->vertex_buffer);
  encode_set_index_buffer(enc, m->index_buffer);
  encode_draw_indexed(enc, m->geometry->indices->len);
}