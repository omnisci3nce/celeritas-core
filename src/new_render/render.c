/**
 * @brief
 */

#include "render.h"
#include "maths_types.h"
#include "pbr.h"
#include "ral_common.h"
#include "render_scene.h"
#include "shadows.h"

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
  struct ResourcePools* resource_pools;
};

bool Renderer_Init(RendererConfig config, Renderer* renderer) {
  // set the RAL backend up

  // create our renderpasses
  Shadow_Init(renderer->shadows);

  return true;
}
