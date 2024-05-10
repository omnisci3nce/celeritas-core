#include "render.h"
#include <glfw3.h>
#include "camera.h"
#include "file.h"
#include "log.h"
#include "ral.h"

/** @brief Creates the pipelines built into Celeritas such as rendering static opaque geometry,
           debug visualisations, immediate mode UI, etc */
void default_pipelines_init(renderer* ren);

bool renderer_init(renderer* ren) {
  // INFO("Renderer init");

  // NOTE: all platforms use GLFW at the moment but thats subject to change
  glfwInit();

#if defined(CEL_REND_BACKEND_OPENGL)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#elif defined(CEL_REND_BACKEND_VULKAN)
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

  // glfw window creation
  GLFWwindow* window = glfwCreateWindow(ren->config.scr_width, ren->config.scr_height,
                                        ren->config.window_name, NULL, NULL);
  if (window == NULL) {
    // ERROR("Failed to create GLFW window\n");
    glfwTerminate();
    return false;
  }
  ren->window = window;

  glfwMakeContextCurrent(ren->window);

  DEBUG("Start gpu backend init");

  if (!gpu_backend_init("Celeritas Engine - Vulkan", window)) {
    FATAL("Couldnt load graphics api backend");
    return false;
  }
  gpu_device_create(&ren->device);  // TODO: handle errors
  gpu_swapchain_create(&ren->swapchain);

  // ren->blinn_phong =
  //     shader_create_separate("assets/shaders/blinn_phong.vert",
  //     "assets/shaders/blinn_phong.frag");

  // ren->skinned =
  //     shader_create_separate("assets/shaders/skinned.vert", "assets/shaders/blinn_phong.frag");

  // default_material_init();

  // Create default rendering pipeline
  /* default_pipelines_init(ren); */

  return true;
}
void renderer_shutdown(renderer* ren) {
  gpu_swapchain_destroy(&ren->swapchain);
  gpu_pipeline_destroy(&ren->static_opaque_pipeline);
  gpu_backend_shutdown();
}

void default_pipelines_init(renderer* ren) {
  // Static opaque geometry
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  gpu_renderpass_desc pass_description = {};
  gpu_renderpass* renderpass = gpu_renderpass_create(&pass_description);

  ren->default_renderpass = *renderpass;

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
  ren->static_opaque_pipeline = *gfx_pipeline;
}

void render_frame_begin(renderer* ren) {
  ren->frame_aborted = false;
  if (!gpu_backend_begin_frame()) {
    ren->frame_aborted = true;
    return;
  }
  gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
  // begin recording
  gpu_cmd_encoder_begin(*enc);
  gpu_cmd_encoder_begin_render(enc, &ren->default_renderpass);
  encode_bind_pipeline(enc, PIPELINE_GRAPHICS, &ren->static_opaque_pipeline);
  encode_set_default_settings(enc);
}
void render_frame_end(renderer* ren) {
  if (ren->frame_aborted) {
    return;
  }
  gpu_temp_draw(3);
  gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
  gpu_cmd_encoder_end_render(enc);
  gpu_cmd_buffer buf = gpu_cmd_encoder_finish(enc);
  gpu_queue_submit(&buf);
  gpu_backend_end_frame();
}
void render_frame_draw(renderer* ren) {}

void gfx_backend_draw_frame(renderer* ren, camera* camera, mat4 model, texture* tex) {}

void geo_set_vertex_colours(geometry_data* geo, vec4 colour) {}
