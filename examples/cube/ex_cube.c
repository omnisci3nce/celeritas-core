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

// Define the shader data
typedef struct mvp_uniforms {
  mat4 model;
  mat4 view;
  mat4 projection;
} mvp_uniforms;
typedef struct my_shader_bind_group {
  mvp_uniforms mvp;
  texture_handle tex;
} my_shader_bind_group;

// We also must create a function that knows how to return a `shader_data_layout`
shader_data_layout mvp_uniforms_layout(void* data) {
  my_shader_bind_group* d = (my_shader_bind_group*)data;
  bool has_data = data != NULL;

  shader_binding b1 = { .label = "mvp_uniforms",
                        .type = SHADER_BINDING_BYTES,
                        .stores_data = has_data,
                        .data = { .bytes = { .size = sizeof(mvp_uniforms) } } };

  shader_binding b2 = { .label = "texture_sampler",
                        .type = SHADER_BINDING_TEXTURE,
                        .stores_data = has_data };
  if (has_data) {
    b1.data.bytes.data = &d->mvp;
    b2.data.texture.handle = d->tex;
  }
  return (shader_data_layout){ .name = "global_ubo", .bindings = { b1, b2 }, .bindings_count = 2 };
}

int main() {
  core_bringup();
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  vec3 camera_pos = vec3(2., 2., 2.);
  vec3 camera_front = vec3_normalise(vec3_negate(camera_pos));
  camera cam = camera_create(camera_pos, camera_front, VEC3_Y, deg_to_rad(45.0));

  // This is how to do it manually; run `static_3d_vertex_description()` to get this built-in vertex
  // format
  vertex_description vertex_input = { .use_full_vertex_size = true };
  vertex_input.debug_label = "Standard Static 3D Vertex Format";
  vertex_desc_add(&vertex_input, "inPosition", ATTR_F32x3);
  vertex_desc_add(&vertex_input, "inNormal", ATTR_F32x3);
  vertex_desc_add(&vertex_input, "inTexCoords", ATTR_F32x2);

  shader_data mvp_uniforms_data = { .data = NULL, .shader_data_get_layout = &mvp_uniforms_layout };

  gpu_renderpass_desc pass_description = {};
  gpu_renderpass* renderpass = gpu_renderpass_create(&pass_description);

  str8 vert_path = str8lit("build/linux/x86_64/debug/cube.vert.spv");
  str8 frag_path = str8lit("build/linux/x86_64/debug/cube.frag.spv");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  struct graphics_pipeline_desc pipeline_description = {
    .debug_name = "Basic Pipeline",
    .vertex_desc = vertex_input,
    .data_layouts = { mvp_uniforms_data },
    .data_layouts_count = 1,
    .vs = { .debug_name = "Triangle Vertex Shader",
            .filepath = vert_path,
            .code = vertex_shader.contents,
            .is_spirv = true },
    .fs = { .debug_name = "Triangle Fragment Shader",
            .filepath = frag_path,
            .code = fragment_shader.contents,
            .is_spirv = true },
    .renderpass = renderpass,
    .wireframe = false,
    .depth_test = false
  };
  gpu_pipeline* gfx_pipeline = gpu_graphics_pipeline_create(pipeline_description);

  // Geometry
  geometry_data cube_data = geo_create_cuboid(f32x3(1, 1, 1));
  mesh cube = mesh_create(&cube_data, false);

  // Texture
  texture_data tex_data = texture_data_load("assets/textures/texture.jpg", false);
  texture_handle texture = texture_data_upload(tex_data, true);

  static f32 theta = 0.0;

  // Main loop
  while (!should_exit(&g_core)) {
    input_update(&g_core.input);

    if (!gpu_backend_begin_frame()) {
      continue;
    }
    gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
    // begin recording
    gpu_cmd_encoder_begin(*enc);
    gpu_cmd_encoder_begin_render(enc, renderpass);
    encode_bind_pipeline(enc, PIPELINE_GRAPHICS, gfx_pipeline);
    encode_set_default_settings(enc);

    theta += 0.01;
    transform transform = { .position = vec3(-0.5, -0.5, -0.5),
                            .rotation = quat_from_axis_angle(VEC3_Y, theta, true),
                            .scale = 1.0 };
    mat4 model = transform_to_mat(&transform);
    mat4 view, proj;
    camera_view_projection(&cam, g_core.renderer.swapchain.extent.width,
                           g_core.renderer.swapchain.extent.height, &view, &proj);
    mvp_uniforms mvp_data = { .model = model, .view = view, .projection = proj };
    my_shader_bind_group shader_bind_data = { .mvp = mvp_data, .tex = texture };
    mvp_uniforms_data.data = &shader_bind_data;
    encode_bind_shader_data(enc, 0, &mvp_uniforms_data);

    // Record draw calls
    draw_mesh(&cube, &model);

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
