#include <glfw3.h>
#include <stdbool.h>

#include "builtin_materials.h"
#include "camera.h"
#include "colours.h"
#include "core.h"
#include "loaders.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "primitives.h"
#include "ral.h"
#include "render.h"
#include "render_types.h"
#include "str.h"

extern core g_core;

#define MODEL_GET(h) (model_pool_get(&g_core.models, h))

const vec3 pointlight_positions[4] = {
  { 10.0 / 1., 10.0 / 1., 10.0 },
  { -10.0 / 1., 10.0 / 1., 10.0 },
  { 10.0 / 1., -10.0 / 1., 10.0 },
  { -10.0 / 1., -10.0 / 1., 10.0 },
};
pbr_point_light point_lights[4];

int main() {
  core_bringup();

  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  for (int i = 0; i < 4; i++) {
    point_lights[i].pos = pointlight_positions[i];
    point_lights[i].color = vec3(300.0, 300.0, 300.0);
  }

  vec3 camera_pos = vec3(3., 2., 0.);
  vec3 camera_front = vec3_normalise(vec3_negate(camera_pos));
  camera cam = camera_create(camera_pos, camera_front, VEC3_NEG_Z, deg_to_rad(45.0));

  shader_data pbr_uniforms = { .data = NULL,
                               .shader_data_get_layout = &pbr_textured_shader_layout };

  // Make the pipeline
  gpu_renderpass_desc pass_description = { .default_framebuffer = true, .color_target = true };
  gpu_renderpass* renderpass = gpu_renderpass_create(&pass_description);
  str8 vert_path = str8lit("assets/shaders/pbr_textured.vert");
  str8 frag_path = str8lit("assets/shaders/pbr_textured.frag");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  struct graphics_pipeline_desc pipeline_description = {
    .debug_name = "PBR Pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = { pbr_uniforms },
    .data_layouts_count = 1,
    .vs = { .debug_name = "PBR (textured) Vertex Shader",
            .filepath = vert_path,
            .code = vertex_shader.contents,
            .is_spirv = false },
    .fs = { .debug_name = "PBR (textured) Fragment Shader",
            .filepath = frag_path,
            .code = fragment_shader.contents,
            .is_spirv = false },
    .renderpass = renderpass,
    .wireframe = false,
    .depth_test = false
  };
  gpu_pipeline* pbr_pipeline = gpu_graphics_pipeline_create(pipeline_description);

  // Model
  model_handle helmet_handle =
      model_load_gltf("assets/models/gltf/DamagedHelmet/glTF/DamagedHelmet.gltf", false);
  INFO("GLTF loaded successfully!");
  model* helmet = MODEL_GET(helmet_handle);

  pbr_textured_bindgroup pbr_bind_data;

  static f32 theta = 0.0;

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

    theta += 0.01;
    transform transform = { .position = vec3(0, 0, 0),
                            .rotation = quat_from_axis_angle(VEC3_Z, theta, true),
                            .scale = 1.0 };
    mat4 model_affine = transform_to_mat(&transform);
    mat4 view, proj;
    camera_view_projection(&cam, 1000, 1000, &view, &proj);

    // Feed shader data
    pbr_bind_data.mvp_matrices =
        (mvp_matrix_uniforms){ .model = model_affine, .view = view, .projection = proj };
    pbr_bind_data.lights = (pbr_params_light_uniforms){
      .viewPos = vec4(cam.position.x, cam.position.y, cam.position.z, 1.0),
      .pointLights = { point_lights[0], point_lights[1], point_lights[2], point_lights[3] }
    };
    pbr_bind_data.textures = (pbr_textures){
      .albedo_tex = helmet->materials->data[0].mat_data.pbr.albedo_map,
      .metal_roughness_tex = helmet->materials->data[0].mat_data.pbr.metallic_map,
      .ao_tex = helmet->materials->data[0].mat_data.pbr.ao_map,
      .normal_tex = helmet->materials->data[0].mat_data.pbr.normal_map,
    };
    pbr_uniforms.data = &pbr_bind_data;
    encode_bind_shader_data(enc, 0, &pbr_uniforms);

    // Record draw calls
    mesh m = helmet->meshes->data[0];
    encode_set_vertex_buffer(enc, m.vertex_buffer);
    encode_set_index_buffer(enc, m.index_buffer);
    encode_draw_indexed(enc, m.geometry->indices->len);

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