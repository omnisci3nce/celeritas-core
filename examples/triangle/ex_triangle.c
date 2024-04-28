#include <glfw3.h>

#include "backend_vulkan.h"
#include "camera.h"
#include "core.h"
#include "file.h"
#include "log.h"
#include "maths.h"
#include "mem.h"
#include "ral.h"
#include "render.h"

// Example setting up a renderer

int main() {
  core* core = core_bringup();
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  gpu_renderpass_desc pass_description = {};
  gpu_renderpass* renderpass = gpu_renderpass_create(&pass_description);

  str8_opt vertex_shader = str8_from_file(&scratch, str8lit("assets/shaders/triangle.vert"));
  str8_opt fragment_shader = str8_from_file(&scratch, str8lit("assets/shaders/triangle.frag"));
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  struct graphics_pipeline_desc pipeline_description = {
    .debug_name = "Basic Pipeline",
    .vs = { .debug_name = "Triangle Vertex Shader",
            .filepath = str8lit("assets/shaders/triangle.vert"),
            .glsl = vertex_shader.contents },
    .fs = { .debug_name = "Triangle Fragment Shader",
            .filepath = str8lit("assets/shaders/triangle.frag"),
            .glsl = fragment_shader.contents },
    .renderpass = renderpass,
    .wireframe = false,
    .depth_test = false
  };
  gpu_pipeline* gfx_pipeline = gpu_graphics_pipeline_create(pipeline_description);

  // Main loop
  while (!should_exit(core)) {
    input_update(&core->input);

    render_frame_begin(&core->renderer);

    static f64 x = 0.0;
    x += 0.01;

    // insert work here

    render_frame_end(&core->renderer);
    glfwSwapBuffers(core->renderer.window);
    glfwPollEvents();
  }

  return 0;
}
