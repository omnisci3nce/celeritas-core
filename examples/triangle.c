// Example demonstrating basic RAL usage by rendering a triangle

#include <celeritas.h>

// static vec4 vertices[] = {
//   {-0.5f, -0.5f, 0.0f, 1.0},
//   { 0.5f, -0.5f, 0.0f, 1.0},
//   { 0.0f,  0.5f, 0.0f, 1.0}
// };

typedef struct VertexData {
  vec4 position;
  vec2 texCoords;
  f32 pad1;
  f32 pad2;
} VertexData;

VertexData squareVertices[] = {
  { { -0.5, -0.5, 0.5, 1.0f }, { 0.0f, 0.0f }, 0.0, 0.0 }, { { -0.5, 0.5, 0.5, 1.0f }, { 0.0f, 1.0f }, 0.0, 0.0 },
  { { 0.5, 0.5, 0.5, 1.0f }, { 1.0f, 1.0f }, 0.0, 0.0 },   { { -0.5, -0.5, 0.5, 1.0f }, { 0.0f, 0.0f }, 0.0, 0.0 },
  { { 0.5, 0.5, 0.5, 1.0f }, { 1.0f, 1.0f }, 0.0, 0.0 },   { { 0.5, -0.5, 0.5, 1.0f }, { 1.0f, 0.0f }, 0.0, 0.0 }
};

pipeline_handle draw_pipeline;
buf_handle tri_vert_buffer;
tex_handle texture;

void draw() {
  render_pass_desc d = {};
  gpu_encoder* enc = ral_render_encoder(d);
  ral_encode_bind_pipeline(enc, draw_pipeline);
  ral_encode_set_vertex_buf(enc, tri_vert_buffer);
  ral_encode_set_texture(enc, texture, 0);
  ral_encode_draw_tris(enc, 0, 6);
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

  // load texture from file
  texture = ral_texture_load_from_file("assets/textures/mc_grass.jpeg");

  // create our buffer to hold vertices
  size_t buffer_size = sizeof(VertexData) * 6;
  printf("size of vertices %ld\n", buffer_size);
  tri_vert_buffer = ral_buffer_create(buffer_size, &squareVertices);

  while (!app_should_exit()) {
    glfwPollEvents();

    ral_frame_start();
    ral_frame_draw(&draw);
    ral_frame_end();
  }

  return 0;
}
