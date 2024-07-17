/**
 * @brief
 */

#include "render.h"
#include <glfw3.h>
#include "camera.h"
#include "colours.h"
#include "core.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "pbr.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render_scene.h"
#include "render_types.h"
#include "shadows.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

extern Core g_core;

struct Renderer {
  struct GLFWwindow* window;
  RendererConfig config;
  GPU_Device device;
  GPU_Swapchain swapchain;
  GPU_Renderpass* default_renderpass;
  bool frame_aborted;
  RenderScene scene;
  PBR_Storage* pbr;
  Shadow_Storage* shadows;
  // Terrain_Storage terrain;
  // Text_Storage text;
  ResourcePools* resource_pools;
};

Renderer* get_renderer() { return g_core.renderer; }

bool Renderer_Init(RendererConfig config, Renderer* ren, GLFWwindow** out_window) {
  INFO("Renderer init");

  // init resource pools
  DEBUG("Initialise GPU resource pools");
  arena pool_arena = arena_create(malloc(1024 * 1024), 1024 * 1024);
  ren->resource_pools = arena_alloc(&pool_arena, sizeof(struct ResourcePools));
  ResourcePools_Init(&pool_arena, ren->resource_pools);

  // GLFW window creation

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

  GLFWwindow* window =
      glfwCreateWindow(config.scr_width, config.scr_height, config.window_name, NULL, NULL);
  if (window == NULL) {
    ERROR("Failed to create GLFW window\n");
    glfwTerminate();
    return false;
  }
  ren->window = window;
  *out_window = window;

  glfwMakeContextCurrent(ren->window);

  // set the RAL backend up
  if (!GPU_Backend_Init(config.window_name, window, ren->resource_pools)) {
    return false;
  }

  GPU_Device_Create(&ren->device);
  GPU_Swapchain_Create(&ren->swapchain);

  // set up default scene
  Camera default_cam =
      Camera_Create(vec3(0.0, 2.0, 4.0), vec3_normalise(vec3(0.0, -2.0, -4.0)), VEC3_Y, 45.0);
  SetCamera(default_cam);
  DirectionalLight default_light = { /* TODO */ };
  SetMainLight(default_light);

  // create our renderpasses
  ren->shadows = malloc(sizeof(Shadow_Storage));
  Shadow_Init(ren->shadows, u32x2(512, 512));

  ren->pbr = malloc(sizeof(PBR_Storage));
  PBR_Init(ren->pbr);

  return true;
}

void Renderer_Shutdown(Renderer* ren) { free(ren->shadows); }
size_t Renderer_GetMemReqs() { return sizeof(Renderer); }

void Render_FrameBegin(Renderer* ren) {
  ren->frame_aborted = false;
  if (!GPU_Backend_BeginFrame()) {
    ren->frame_aborted = true;
    WARN("Frame aborted");
    return;
  }
}
void Render_FrameEnd(Renderer* ren) {
  if (ren->frame_aborted) {
    return;
  }

  GPU_CmdEncoder* enc = GPU_GetDefaultEncoder();

  GPU_Backend_EndFrame();
}
void Render_RenderEntities(RenderEnt* entities, size_t entity_count) {
  Renderer* ren = get_renderer();
  RenderScene scene = ren->scene;

  // TOOD: -- Shadows
  // f32 near_plane = 1.0, far_plane = 10.0;
  // Mat4 light_projection = mat4_orthographic(-10.0, 10.0, -10.0, 10.0, near_plane, far_plane);
  // Vec3 pos = vec3_negate(scene.sun.direction);
  // Mat4 light_view = mat4_look_at(pos, VEC3_ZERO, VEC3_Y);
  // Mat4 light_space_matrix = mat4_mult(light_view, light_projection);
  // Shadow_ShadowmapExecute(ren->shadows, light_space_matrix, entities, entity_count);
}

TextureData TextureDataLoad(const char* path, bool invert_y) {
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
  GPU_TextureFormat format;
  if (num_channels == 4) {
    channel_type = GL_RGBA;
    format = TEXTURE_FORMAT_8_8_8_8_RGBA_UNORM;
  } else {
    channel_type = GL_RGB;
    format = TEXTURE_FORMAT_8_8_8_RGB_UNORM;
  }
  TextureDesc desc = { .extents = { width, height },
                       .format = format,
                       .tex_type = TEXTURE_TYPE_2D };

  return (TextureData){ .description = desc, .image_data = data };
}

TextureHandle TextureLoadFromFile(const char* path) {
  TextureData tex_data = TextureDataLoad(path, false);
  TextureHandle h = GPU_TextureCreate(tex_data.description, true, tex_data.image_data);
  return h;
}

Mesh Mesh_Create(Geometry* geometry, bool free_on_upload) {
  Mesh m = { 0 };

  // Create and upload vertex buffer
  size_t vert_bytes = geometry->vertices->len * sizeof(Vertex);
  INFO("Creating vertex buffer with size %d (%d x %d)", vert_bytes, geometry->vertices->len,
       sizeof(Vertex));
  m.vertex_buffer =
      GPU_BufferCreate(vert_bytes, BUFFER_VERTEX, BUFFER_FLAG_GPU, geometry->vertices->data);

  // Create and upload index buffer
  size_t index_bytes = geometry->indices->len * sizeof(u32);
  INFO("Creating index buffer with size %d (len: %d)", index_bytes, geometry->indices->len);
  m.index_buffer =
      GPU_BufferCreate(index_bytes, BUFFER_INDEX, BUFFER_FLAG_GPU, geometry->indices->data);

  m.is_uploaded = true;
  m.geometry = geometry;
  if (free_on_upload) {
    Geometry_Destroy(geometry);
  }
  return m;
}

void Geometry_Destroy(Geometry* geometry) {
  if (geometry->indices) {
    u32_darray_free(geometry->indices);
  }
  if (geometry->vertices) {
    Vertex_darray_free(geometry->vertices);
  }
}

void SetCamera(Camera camera) { g_core.renderer->scene.camera = camera; }
void SetMainLight(DirectionalLight light) { g_core.renderer->scene.sun = light; }