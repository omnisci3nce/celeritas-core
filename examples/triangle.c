// Example demonstrating basic RAL usage by rendering a triangle

#include <celeritas.h>

static vec4 vertices[] = {
  {-0.5f, -0.5f, 0.0f, 1.0},
  { 0.5f, -0.5f, 0.0f, 1.0},
  { 0.0f,  0.5f, 0.0f, 1.0}
};

pipeline_handle draw_pipeline;
buf_handle tri_vert_buffer;

void draw() {
  render_pass_desc d = {};
  gpu_encoder* enc = ral_render_encoder(d);
  ral_encode_bind_pipeline(enc, draw_pipeline);
  ral_encode_set_vertex_buf(enc, tri_vert_buffer);
  ral_encode_draw_tris(enc, 0, 3);
  ral_encoder_finish_and_submit(enc);
}

int main() {
  core_bringup("Celeritas Example: Triangle", NULL);

  // create rendering pipeline
  gfx_pipeline_desc pipeline_desc = {
    .label = "Triangle drawing pipeline",
    .vertex_desc = NULL, // TODO
    .vertex = {
      .source = NULL,
      .is_spirv = false,
      .entry_point = "vertexShader",
      .stage = STAGE_VERTEX,
    },
    .fragment = {
      .source = NULL,
      .is_spirv = false,
      .entry_point = "fragmentShader",
      .stage = STAGE_FRAGMENT,
    },
  };

  draw_pipeline = ral_gfx_pipeline_create(pipeline_desc);

  // create our buffer to hold vertices
  printf("size of vertices %ld\n", sizeof(vec4) * 3);
  tri_vert_buffer = ral_buffer_create(sizeof(vec4) * 3, &vertices);

  while (!app_should_exit()) {
    glfwPollEvents();

    ral_frame_start();
    ral_frame_draw(&draw);
    ral_frame_end();
  }

  return 0;
}
