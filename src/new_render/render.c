/**
 * @brief
 */

#include "render.h"
#include <assert.h>
#include <glfw3.h>
#include "camera.h"
#include "core.h"
#include "log.h"
#include "maths.h"
#include "mem.h"
#include "pbr.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render_scene.h"
#include "render_types.h"
#include "shadows.h"
#include "terrain.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define FRAME_ARENA_SIZE MB(1)

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
  Terrain_Storage* terrain;
  // Text_Storage text;
  ResourcePools* resource_pools;
  arena frame_arena;
  TextureHandle white_1x1;
};

Renderer* get_renderer() { return g_core.renderer; }

bool Renderer_Init(RendererConfig config, Renderer* ren, GLFWwindow** out_window,
                   GLFWwindow* optional_window) {
  INFO("Renderer init");

  ren->frame_arena = arena_create(malloc(FRAME_ARENA_SIZE), FRAME_ARENA_SIZE);

  // init resource pools
  DEBUG("Initialise GPU resource pools");
  arena pool_arena = arena_create(malloc(1024 * 1024), 1024 * 1024);
  ren->resource_pools = arena_alloc(&pool_arena, sizeof(struct ResourcePools));
  ResourcePools_Init(&pool_arena, ren->resource_pools);

  // GLFW window creation
  GLFWwindow* window;
  if (optional_window != NULL) {
    INFO("GLFWwindow pointer was provided!!!! Skipping generic glfw init..");
    window = optional_window;
  } else {
    INFO("No GLFWwindow provided - creating one");
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

    window = glfwCreateWindow(config.scr_width, config.scr_height, config.window_name, NULL, NULL);
    INFO("Window created");
    if (window == NULL) {
      ERROR("Failed to create GLFW window\n");
      glfwTerminate();
      return false;
    }
  }

  // #if defined(CEL_REND_BACKEND_OPENGL)
  //   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  //   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  //   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // #elif defined(CEL_REND_BACKEND_VULKAN)
  //   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // #endif

  ren->window = window;
  *out_window = window;

  glfwMakeContextCurrent(ren->window);

  // FIXME
  // DEBUG("Set up GLFW window callbacks");
  glfwSetWindowSizeCallback(window, Render_WindowSizeChanged);

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
  // Shadow_Init(ren->shadows, 1024, 1024);

  ren->pbr = malloc(sizeof(PBR_Storage));
  PBR_Init(ren->pbr);

  ren->terrain = malloc(sizeof(Terrain_Storage));
  // Terrain_Init(ren->terrain);

  // load default textures
  ren->white_1x1 = TextureLoadFromFile("assets/textures/white1x1.png");

  return true;
}

void Renderer_Shutdown(Renderer* ren) {
  free(ren->shadows);
  DEBUG("Freed Shadows storage");
  free(ren->pbr);
  DEBUG("Freed PBR storage");
  free(ren->terrain);
  DEBUG("Freed Terrain storage");
  arena_free_storage(&ren->frame_arena);
  DEBUG("Freed frame allocator buffer");
}
size_t Renderer_GetMemReqs() { return sizeof(Renderer); }

void Render_WindowSizeChanged(GLFWwindow* window, i32 new_width, i32 new_height) {
  (void)window;
  INFO("Window size changed callback");
  // Renderer* ren = Core_GetRenderer(&g_core);
  GPU_Swapchain_Resize(new_width, new_height);
}

void Render_FrameBegin(Renderer* ren) {
  arena_free_all(&ren->frame_arena);
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

  Shadow_Storage* shadow_storage = Render_GetShadowStorage();
  shadow_storage->enabled = false;
  TextureHandle depthmap =
      shadow_storage->enabled ? Shadow_GetShadowMapTexture(shadow_storage) : INVALID_TEX_HANDLE;

  PBR_Execute(ren->pbr, scene.camera, depthmap, entities, entity_count);
}

TextureData TextureDataLoad(const char* path, bool invert_y) {
  TRACE("Load texture %s", path);

  // load the file data
  int width, height, num_channels;
  stbi_set_flip_vertically_on_load(invert_y);

#pragma GCC diagnostic ignored "-Wpointer-sign"
  char* data = stbi_load(path, &width, &height, &num_channels, 0);
  if (data) {
    DEBUG("loaded texture: %s", path);
  } else {
    WARN("failed to load texture");
  }

  // printf("width: %d height: %d num channels: %d\n", width, height, num_channels);

  unsigned int channel_type;
  GPU_TextureFormat format;
  if (num_channels == 4) {
    channel_type = GL_RGBA;
    format = TEXTURE_FORMAT_8_8_8_8_RGBA_UNORM;
  } else {
    channel_type = GL_RGB;
    format = TEXTURE_FORMAT_8_8_8_RGB_UNORM;
  }
  TextureDesc desc = {
    .extents = { width, height },
    .format = format,
    .num_channels = num_channels,
    .tex_type = TEXTURE_TYPE_2D,
  };

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
  if (geometry->has_indices) {
    size_t index_bytes = geometry->indices->len * sizeof(u32);
    INFO("Creating index buffer with size %d (len: %d)", index_bytes, geometry->indices->len);
    m.index_buffer =
        GPU_BufferCreate(index_bytes, BUFFER_INDEX, BUFFER_FLAG_GPU, geometry->indices->data);
  }

  m.is_uploaded = true;
  m.geometry = *geometry;  // clone geometry data and store on Mesh struct
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

arena* GetRenderFrameArena(Renderer* r) { return &r->frame_arena; }

RenderScene* Render_GetScene() {
  Renderer* ren = Core_GetRenderer(&g_core);
  return &ren->scene;
}

Shadow_Storage* Render_GetShadowStorage() {
  Renderer* ren = Core_GetRenderer(&g_core);
  return ren->shadows;
}

Terrain_Storage* Render_GetTerrainStorage() {
  Renderer* ren = Core_GetRenderer(&g_core);
  return ren->terrain;
}

TextureHandle Render_GetWhiteTexture() {
  Renderer* ren = Core_GetRenderer(&g_core);
  return ren->white_1x1;
}

arena* Render_GetFrameArena() {
  Renderer* ren = Core_GetRenderer(&g_core);
  return &ren->frame_arena;
}