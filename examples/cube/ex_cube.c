#include <glfw3.h>

#include "buf.h"
#include "camera.h"
#include "core.h"
#include "file.h"
#include "log.h"
#include "maths.h"
#include "mem.h"
#include "ral.h"
#include "ral_types.h"
#include "render.h"

extern core g_core;

// Define the shader data
typedef struct mvp_uniforms {
  mat4 model;
  mat4 view;
  mat4 projection;
} mvp_uniforms;

shader_data_layout mvp_uniforms_layout(void* data) {
  mvp_uniforms* d = (mvp_uniforms*)data;
  bool has_data = data != NULL;
  
  shader_binding b1 = {
    .label = "model",
    .type = SHADER_BINDING_BYTES,
    .stores_data = has_data,
    .data = {.bytes = { .size = sizeof(mat4) }}
  };
  shader_binding b2 = {
    .label = "view",
    .type = SHADER_BINDING_BYTES,
    .stores_data = has_data,
    .data = {.bytes = { .size = sizeof(mat4) }}
  };
  shader_binding b3 = {
    .label = "projection",
    .type = SHADER_BINDING_BYTES,
    .stores_data = has_data,
    .data = {.bytes = { .size = sizeof(mat4) }}
  };
  if (has_data) {
     b1.data.bytes.data = &d->model;
     b2.data.bytes.data = &d->view;
     b3.data.bytes.data = &d->projection;
  }
  return (shader_data_layout ){.name = "mvp_uniforms", .bindings = {
    b1, b2, b3
  }};
}

int main() {
  core_bringup();
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  DEBUG("render capacity %d", g_core.default_scene.renderables->capacity);

  shader_data_layout mvp_layout = mvp_uniforms_layout(NULL);

  mvp_uniforms mvp_data = {
    .model = mat4_ident(),
    .view = mat4_ident(),
    .projection = mat4_ident()
  };

  shader_data mvp_uniforms_data = {
    .data = &mvp_data,
    .shader_data_get_layout = &mvp_uniforms_layout
  };

  gpu_renderpass_desc pass_description = {};
  gpu_renderpass* renderpass = gpu_renderpass_create(&pass_description);

  str8 vert_path = str8lit("build/linux/x86_64/debug/triangle.vert.spv");
  str8 frag_path = str8lit("build/linux/x86_64/debug/triangle.frag.spv");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  struct graphics_pipeline_desc pipeline_description = {
    .debug_name = "Basic Pipeline",
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

  buffer_handle triangle_vert_buf =
      gpu_buffer_create(sizeof(vertices), CEL_BUFFER_VERTEX, CEL_BUFFER_FLAG_GPU, vertices);

  buffer_handle triangle_index_buf =
      gpu_buffer_create(sizeof(indices), CEL_BUFFER_INDEX, CEL_BUFFER_FLAG_GPU, indices);

  // Main loop
  while (!should_exit(&g_core)) {
    glfwPollEvents();
    input_update(&g_core.input);

    // render_frame_begin(&g_core.renderer);

    if (!gpu_backend_begin_frame()) {
      continue;
    }
    gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
    // begin recording
    gpu_cmd_encoder_begin(*enc);
    gpu_cmd_encoder_begin_render(enc, renderpass);
    encode_bind_pipeline(enc, PIPELINE_GRAPHICS, gfx_pipeline);
    encode_set_default_settings(enc);

    // Record draw calls
    encode_set_vertex_buffer(enc, triangle_vert_buf);
    encode_set_index_buffer(enc, triangle_index_buf);
    encode_bind_shader_data(enc, 0, &mvp_uniforms_data);
    gpu_temp_draw(6);

    // End recording
    gpu_cmd_encoder_end_render(enc);

    gpu_cmd_buffer buf = gpu_cmd_encoder_finish(enc);
    gpu_queue_submit(&buf);
    // Submit
    gpu_backend_end_frame();

    // render_frame_end(&g_core.renderer);
    // glfwSwapBuffers(core->renderer.window);
  }

  renderer_shutdown(&g_core.renderer);

  return 0;
}
