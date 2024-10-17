#include <celeritas.h>

// vertices

pipeline_handle draw_pipeline;
buf_handle cube_vbuf;
tex_handle texture;

void draw() {
  render_pass_desc d = {};
  gpu_encoder* enc = ral_render_encoder(d);
  ral_encode_bind_pipeline(enc, draw_pipeline);
  ral_encode_set_vertex_buf(enc, cube_vbuf);
  ral_encode_set_texture(enc, texture, 0);
  ral_encode_draw_tris(enc, 0, 36);
  ral_encoder_finish_and_submit(enc);
}

int main() {
  core_bringup("Celeritas Example: Triangle", NULL);

  // create rendering pipeline
  gfx_pipeline_desc pipeline_desc = {
    .label = "Textured cube pipeline",
    .vertex_desc = static_3d_vertex_format(),
    .vertex = {
      .source = NULL,
      .is_spirv = false,
      .entry_point = "cubeVertexShader",
      .stage = STAGE_VERTEX,
    },
    .fragment = {
      .source = NULL,
      .is_spirv = false,
      .entry_point = "cubeFragmentShader",
      .stage = STAGE_FRAGMENT,
    },
  };

  draw_pipeline = ral_gfx_pipeline_create(pipeline_desc);

  // create the cube geometry
  geometry cube = geo_cuboid(1.0, 1.0, 1.0);

  // upload vertex data to the gpu
  cube_vbuf = ral_buffer_create(64 * 36, cube.vertex_data);

  while (!app_should_exit()) {
    glfwPollEvents();

    ral_frame_start();
    ral_frame_draw(&draw);
    ral_frame_end();
  }

  return 0;
}