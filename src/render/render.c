#include <glfw3.h>
#include "maths_types.h"
#include "render_types.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "camera.h"
#include "file.h"
#include "log.h"
#include "mem.h"
#include "ral.h"
#include "ral_types.h"
#include "render.h"

//---NEW
#include "static_pipeline.h"
//---END

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

  DEBUG("Set up GLFW window callbacks");

  DEBUG("Start gpu backend init");

  if (!gpu_backend_init("Celeritas Engine - Vulkan", window)) {
    FATAL("Couldnt load graphics api backend");
    return false;
  }
  gpu_device_create(&ren->device);  // TODO: handle errors
  gpu_swapchain_create(&ren->swapchain);

  DEBUG("Initialise GPU resource pools");
  arena pool_arena = arena_create(malloc(1024 * 1024), 1024 * 1024);
  ren->resource_pools = arena_alloc(&pool_arena, sizeof(struct resource_pools));
  resource_pools_init(&pool_arena, ren->resource_pools);

  // Create default rendering pipeline
  default_pipelines_init(ren);

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

  printf("Load shaders\n");
  str8 vert_path, frag_path;
#ifdef CEL_REND_BACKEND_OPENGL
  vert_path = str8lit("assets/shaders/cube.vert");
  frag_path = str8lit("assets/shaders/cube.frag");
#else
  vert_path = str8lit("build/linux/x86_64/debug/cube.vert.spv");
  frag_path = str8lit("build/linux/x86_64/debug/cube.frag.spv");
#endif
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  // Vertex attributes
  vertex_description vertex_input = { 0 };
  vertex_input.debug_label = "Standard Static 3D Vertex Format";
  vertex_desc_add(&vertex_input, "inPosition", ATTR_F32x3);
  vertex_desc_add(&vertex_input, "inNormal", ATTR_F32x3);
  vertex_desc_add(&vertex_input, "inTexCoords", ATTR_F32x2);
  vertex_input.use_full_vertex_size = true;

  // Shader data bindings
  shader_data mvp_uniforms_data = { .data = NULL, .shader_data_get_layout = &mvp_uniforms_layout };

  struct graphics_pipeline_desc pipeline_description = {
    .debug_name = "Basic Pipeline",
    .vertex_desc = vertex_input,
    .data_layouts = { mvp_uniforms_data },
    .data_layouts_count = 1,
    .vs = { .debug_name = "Basic Vertex Shader",
            .filepath = vert_path,
            .code = vertex_shader.contents,
            .is_spirv = true },
    .fs = { .debug_name = "Basic Fragment Shader",
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
    WARN("Frame aborted");
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
  gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
  gpu_cmd_encoder_end_render(enc);
  gpu_cmd_buffer buf = gpu_cmd_encoder_finish(enc);
  gpu_queue_submit(&buf);
  gpu_backend_end_frame();
}
void render_frame_draw(renderer* ren) {}

bool mesh_has_indices(mesh* m) { return m->geometry->has_indices; }

/**
 *
 * @param Camera used for getting the view projection matric to draw the mesh with.
 *        If NULL use the last used camera  */
void draw_mesh(mesh* mesh, mat4* model, camera* cam) {  // , mat4* view, mat4* proj) {
  gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();

  encode_set_vertex_buffer(enc, mesh->vertex_buffer);
  if (mesh_has_indices(mesh)) {
    encode_set_index_buffer(enc, mesh->index_buffer);
  }

  mat4 view, proj;
  if (cam) {
    camera_view_projection(cam,  // FIXME: proper swapchain dimensions
                           1000, 1000, &view, &proj);

  } else {
    WARN("No camera set");
  }
  mvp_uniforms mvp_data = { .model = *model, .view = view, .projection = proj };
  my_shader_bind_group shader_bind_data = { .mvp = mvp_data };
  shader_data mvp_uniforms_data = { .data = &shader_bind_data,
                                    .shader_data_get_layout = &mvp_uniforms_layout };
  encode_bind_shader_data(enc, 0, &mvp_uniforms_data);

  encode_draw_indexed(enc, mesh->geometry->indices->len);
}

void gfx_backend_draw_frame(renderer* ren, camera* camera, mat4 model, texture* tex) {}

void geo_set_vertex_colours(geometry_data* geo, vec4 colour) {}

// --- NEW

mesh mesh_create(geometry_data* geometry, bool free_on_upload) {
  mesh m = { 0 };

  // Create and upload vertex buffer
  size_t vert_bytes = geometry->vertices->len * sizeof(vertex);
  INFO("Creating vertex buffer with size %d (%d x %d)", vert_bytes, geometry->vertices->len,
       sizeof(vertex));
  m.vertex_buffer = gpu_buffer_create(vert_bytes, CEL_BUFFER_VERTEX, CEL_BUFFER_FLAG_GPU,
                                      geometry->vertices->data);

  // Create and upload index buffer
  size_t index_bytes = geometry->indices->len * sizeof(u32);
  INFO("Creating index buffer with size %d (len: %d)", index_bytes, geometry->indices->len);
  m.index_buffer = gpu_buffer_create(index_bytes, CEL_BUFFER_INDEX, CEL_BUFFER_FLAG_GPU,
                                     geometry->indices->data);

  m.is_uploaded = true;
  // m.has_indices = geometry->has_indices;
  // m.index_count = geometry->indices.len;
  m.geometry = geometry;
  if (free_on_upload) {
    geo_free_data(geometry);
  }

  // TODO: materials?

  return m;
}

// --- Textures

texture_data texture_data_load(const char* path, bool invert_y) {
  TRACE("Load texture %s", path);

  // load the file data
  int width, height, num_channels;
  stbi_set_flip_vertically_on_load(invert_y);

#pragma GCC diagnostic ignored "-Wpointer-sign"
  char* data = stbi_load(path, &width, &height, &num_channels, STBI_rgb_alpha);
  if (data) {
    DEBUG("loaded texture: %s", path);
  } else {
    WARN("failed to load texture");
  }

  unsigned int channel_type;
  if (num_channels == 4) {
    channel_type = GL_RGBA;
  } else {
    channel_type = GL_RGB;
  }
  texture_desc desc = { .extents = { width, height },
                        .format = CEL_TEXTURE_FORMAT_8_8_8_8_RGBA_UNORM,
                        .tex_type = CEL_TEXTURE_TYPE_2D };

  return (texture_data){ .description = desc, .image_data = data };
}

texture_handle texture_data_upload(texture_data data, bool free_on_upload) {
  texture_handle handle = gpu_texture_create(data.description, true, data.image_data);
  if (free_on_upload) {
    TRACE("Freed stb_image data");
    stbi_image_free(data.image_data);
  }
  return handle;
}

/** @brief load all of the texture for a PBR material and returns an unnamed material */
material pbr_material_load(char* albedo_path, char* normal_path, bool metal_roughness_combined,
                           char* metallic_path, char* roughness_map, char* ao_map) {
  material m = { 0 };
  m.kind = MAT_PBR;

  // For now we must have the required textures
  assert(albedo_path);
  assert(normal_path);
  assert(metallic_path);
  assert(metal_roughness_combined);

  m.mat_data.pbr.metal_roughness_combined = metal_roughness_combined;
  texture_data tex_data;
  tex_data = texture_data_load(albedo_path, false);
  m.mat_data.pbr.albedo_map = texture_data_upload(tex_data, true);
  tex_data = texture_data_load(normal_path, false);
  m.mat_data.pbr.normal_map = texture_data_upload(tex_data, true);
  tex_data = texture_data_load(metallic_path, false);
  m.mat_data.pbr.metallic_map = texture_data_upload(tex_data, true);

  return m;
}
