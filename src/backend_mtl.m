#define GPU_METAL 1

#ifdef GPU_METAL
#include <celeritas.h>

#define MTL_DEBUG_LAYER 1

// Obj-C imports
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

#define GLFW_INCLUDE_NONE
#import <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#import <GLFW/glfw3native.h>

// --- RAL types

struct gpu_swapchain {
  int width, height;
  CAMetalLayer* swapchain;
};

typedef struct metal_context {
  GLFWwindow* window;
  NSWindow* metal_window;

  id<MTLDevice> device;
  id<CAMetalDrawable> surface;
  gpu_swapchain default_swapchain;

  id<MTLCommandQueue> command_queue;
} metal_context;

static metal_context ctx;

void ral_backend_init(const char* window_name, struct GLFWwindow* window) {
  printf("loading Metal backend\n");

  printf("gpu device creation\n");
  const id<MTLDevice> gpu = MTLCreateSystemDefaultDevice();
  ctx.device = gpu;

  printf("window init\n");
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwMakeContextCurrent(window);
  NSWindow* nswindow = glfwGetCocoaWindow(window);
  ctx.metal_window = nswindow;

  // effectively the "framebuffer"
  CAMetalLayer* metal_layer = [CAMetalLayer layer];
  metal_layer.device = gpu;
  metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  ctx.metal_window.contentView.layer = metal_layer;
  ctx.metal_window.contentView.wantsLayer = true;

  printf("command queue creation\n");
  const id<MTLCommandQueue> queue = [ctx.device newCommandQueue];
  ctx.command_queue = queue;
}

void ral_backend_shutdown() {
  // no-op
}

#endif