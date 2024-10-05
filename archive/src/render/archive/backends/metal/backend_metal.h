#pragma once
// #define CEL_REND_BACKEND_METAL
#if defined(CEL_REND_BACKEND_METAL)

#include "defines.h"
#include "maths_types.h"
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>
#else
typedef void* id;
#endif

typedef struct gpu_swapchain {
  u32x2 dimensions;
#ifdef __OBJC__
  CAMetalLayer* swapchain;
#else
  void* swapchain;
#endif
} gpu_swapchain;
typedef struct gpu_device {
/** @brief `device` gives us access to our GPU */
#ifdef __OBJC__
  id<MTLDevice> id;
#else
  void* id;
#endif
} gpu_device;
typedef struct gpu_pipeline_layout {
  void* pad;
} gpu_pipeline_layout;
typedef struct gpu_pipeline {
#ifdef __OBJC__
  id<MTLRenderPipelineState> pipeline_state;
#else
  void* pipeline_state;
#endif
} gpu_pipeline;
typedef struct gpu_renderpass {
#ifdef __OBJC__
  MTLRenderPassDescriptor* rpass_descriptor;
#else
  void* rpass_descriptor;
#endif
} gpu_renderpass;
typedef struct gpu_cmd_encoder {
#ifdef __OBJC__
  id<MTLCommandBuffer> cmd_buffer;
  id<MTLRenderCommandEncoder> render_encoder;
#else
  void* cmd_buffer;
  void* render_encoder;
#endif
} gpu_cmd_encoder;
typedef struct gpu_cmd_buffer {
  void* pad;
} gpu_cmd_buffer;

typedef struct gpu_buffer {
#ifdef __OBJC__
  id<MTLBuffer> id;
#else
  void* id;
#endif
  u64 size;
} gpu_buffer;
typedef struct gpu_texture {
  void* pad;
} gpu_texture;

#endif