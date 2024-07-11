/**
 * @brief
 */

#include "render.h"
#include "maths_types.h"
#include "shadow.h"

struct RendererConfig {
    char window_name[256];
    u32 scr_width, scr_height;
    Vec3 clear_colour;
};

struct Renderer {
  struct GLFWwindow* window;
    RendererConfig config;
    GPU_Device device;
    GPU_Swapchain swapchain;
    GPU_Renderpass* default_renderpass;
    bool frame_aborted;
    RenderScene scene;
    PBR_Storage pbr;
    Shadow_Storage shadows;
    Terrain_Storage terrain;
    Text_Storage text;
    ResourcePools* resource_pools;
};

bool Renderer_Init(RendererConfig config, Renderer* renderer) {
    // set the RAL backend up

    // create our renderpasses
    Shadow_Init(&renderer->shadows);

    return true;
}
