#define GPU_METAL 1

#ifdef GPU_METAL
#include <celeritas.h>

// Obj-C imports
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

// --- RAL types
struct gpu_device {
  id<MTLDevice> id;
};

struct gpu_swapchain {
  int width, height;
  CAMetalLayer* swapchain;
};

#endif