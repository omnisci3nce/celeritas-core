#include <glfw3.h>

#include "backend_vulkan.h"
#include "buf.h"
#include "camera.h"
#include "core.h"
#include "defines.h"
#include "file.h"
#include "log.h"
#include "maths.h"
#include "mem.h"
#include "ral.h"
#include "ral_types.h"
#include "render.h"
#include "render_types.h"

extern core g_core;

int main() {
  custom_vertex vertices[] = {
    (custom_vertex){ .pos = vec2(-0.5, -0.5), .color = vec3(1.0, 1.0, 1.0) },
    (custom_vertex){ .pos = vec2(0.5, -0.5), .color = vec3(1.0, 0.0, 0.0) },
    (custom_vertex){ .pos = vec2(-0.5, 0.5), .color = vec3(0.0, 0.0, 1.0) },
    (custom_vertex){ .pos = vec2(0.5, 0.5), .color = vec3(0.0, 1.0, 0.0) },
  };
  const u32 indices[] = { 2, 1, 0, 1, 2, 3 };

  core_bringup();
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  vertex_description vertex_input = { .use_full_vertex_size = false };
  vertex_input.debug_label = "Hello";
  vertex_desc_add(&vertex_input, "inPos", ATTR_F32x2);
  vertex_desc_add(&vertex_input, "inColor", ATTR_F32x3);

  gpu_renderpass_desc pass_description = {};
  gpu_renderpass* renderpass = gpu_renderpass_create(&pass_description);

  str8 vert_path, frag_path;
#if defined(CEL_REND_BACKEND_OPENGL)
  vert_path = str8lit("assets/shaders/triangle.vert");
  frag_path = str8lit("assets/shaders/triangle.frag");
#elif defined(CEL_REND_BACKEND_METAL)
  vert_path = str8lit("build/gfx.metallib");
  frag_path = str8lit("build/gfx.metallib");
#endif
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  struct graphics_pipeline_desc pipeline_description = {
    .debug_name = "Basic Pipeline",
    .vertex_desc = vertex_input,
    // .data_layouts = {0},
    // .data_layouts_count = 0,
    .vs = { .debug_name = "Triangle Vertex Shader",
            .filepath = vert_path,
            .code = vertex_shader.contents,
#ifdef CEL_REND_BACKEND_METAL
            .is_combined_vert_frag = true,
#endif
            .is_spirv = true,
    },
    .fs = { .debug_name = "Triangle Fragment Shader",
            .filepath = frag_path,
            .code = fragment_shader.contents,
            .is_spirv = true },
    .renderpass = renderpass,
    .wireframe = false,
    .depth_test = false
  };
  gpu_pipeline* gfx_pipeline = gpu_graphics_pipeline_create(pipeline_description);

  // Load triangle vertex and index data
  buffer_handle triangle_vert_buf = gpu_buffer_create(4 * sizeof(custom_vertex), CEL_BUFFER_VERTEX,
                                                      CEL_BUFFER_FLAG_GPU, vertices);

  buffer_handle triangle_index_buf =
      gpu_buffer_create(sizeof(indices), CEL_BUFFER_INDEX, CEL_BUFFER_FLAG_GPU, indices);

  // // Main loop
  while (!should_exit()) {
    input_update(&g_core.input);

    if (!gpu_backend_begin_frame()) {
      continue;
    }
    gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
    //   // Begin recording
    gpu_cmd_encoder_begin(*enc);
    gpu_cmd_encoder_begin_render(enc, renderpass);
    encode_bind_pipeline(enc, PIPELINE_GRAPHICS, gfx_pipeline);
    encode_set_default_settings(enc);

    //   // Record draw calls
    encode_set_vertex_buffer(enc, triangle_vert_buf);
    encode_set_index_buffer(enc, triangle_index_buf);
    encode_draw_indexed(enc, 6);

    //   // End recording
    //   gpu_cmd_encoder_end_render(enc);

    //   gpu_cmd_buffer buf = gpu_cmd_encoder_finish(enc);  // Command buffer is no longer recording
    //   and is ready to submit
    //   // Submit
    //   gpu_queue_submit(&buf);
    gpu_backend_end_frame();
  }

  renderer_shutdown(&g_core.renderer);

  return 0;
}
