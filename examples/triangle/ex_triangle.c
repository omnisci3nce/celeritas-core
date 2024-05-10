#include <glfw3.h>

#include "backend_vulkan.h"
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

// Example setting up a renderer

extern core g_core;

const custom_vertex vertices[] = {
  (custom_vertex){ .pos = vec2(-0.5, 0.5), .color = vec3(0.0, 0.0, 1.0) },
  (custom_vertex){ .pos = vec2(0.5, 0.5), .color = vec3(0.0, 1.0, 0.0) },
  (custom_vertex){ .pos = vec2(0.0, -0.5), .color = vec3(1.0, 0.0, 0.0) },
};
_Static_assert(sizeof(vertices) == (5 * 3 * sizeof(float)), "");

int main() {
  core_bringup();
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  DEBUG("render capacity %d", g_core.default_scene.renderables->capacity);

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

  buffer_handle triangle_vert_buf = gpu_buffer_create(
      3 * sizeof(custom_vertex), CEL_BUFFER_VERTEX, CEL_BUFFER_FLAG_GPU,
      vertices);  // gpu_buffer_create(3 * sizeof(custom_vertex), CEL_BUFFER_VERTEX);
  /* buffer_upload_bytes(triangle_vert_buf, (bytebuffer){ .buf = vertices, .size = sizeof(vertices)
   * }, */
  /*                     0, sizeof(vertices)); */

  // Main loop
  while (!should_exit(&g_core)) {
    glfwPollEvents();
    input_update(&g_core.input);

    // render_frame_begin(&g_core.renderer);

    static f64 x = 0.0;
    x += 0.01;

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
    gpu_temp_draw(3);

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
