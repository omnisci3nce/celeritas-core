#include <assert.h>
// #define CEL_REND_BACKEND_METAL
#if defined(CEL_REND_BACKEND_METAL)
#include <stddef.h>
#include "ral_types.h"
#include "colours.h"
#include <stdlib.h>
#include "camera.h"
#include "defines.h"
#include "file.h"
#include "log.h"
#include "maths_types.h"
#include "ral.h"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>
#include "backend_metal.h"

// --- Handy macros
#define BUFFER_GET(h) (buffer_pool_get(&context.resource_pools->buffers, h))
#define TEXTURE_GET(h) (texture_pool_get(&context.resource_pools->textures, h))

typedef struct metal_context {
  GLFWwindow* window;
  NSWindow* metal_window;
  arena pool_arena;

  gpu_device* device;
  gpu_swapchain* swapchain;
  id<CAMetalDrawable> surface;

  id<MTLCommandQueue> command_queue;
  gpu_cmd_encoder main_command_buf;
  gpu_backend_pools gpu_pools;
  struct resource_pools* resource_pools;
} metal_context;

static metal_context context;

struct GLFWwindow;

bool gpu_backend_init(const char *window_name, struct GLFWwindow *window) {
  INFO("loading Metal backend");

  memset(&context, 0, sizeof(metal_context));
  context.window = window;

 size_t pool_buffer_size = 1024 * 1024;
  context.pool_arena = arena_create(malloc(pool_buffer_size), pool_buffer_size);

  backend_pools_init(&context.pool_arena, &context.gpu_pools);
  context.resource_pools = malloc(sizeof(struct resource_pools));
  resource_pools_init(&context.pool_arena, context.resource_pools);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  glfwMakeContextCurrent(window);
  // FIXME: glfwSetFramebufferSizeCallback(ren->window, framebuffer_size_callback);

  // get a NSWindow pointer from GLFWwindow
  NSWindow *nswindow = glfwGetCocoaWindow(window);
  context.metal_window = nswindow;

  // const id<MTLCommandQueue> queue = [gpu newCommandQueue];
  // CAMetalLayer *swapchain = [CAMetalLayer layer];
  // swapchain.device = gpu;
  // swapchain.opaque = YES;

  // // set swapchain for the window
  // nswindow.contentView.layer = swapchain;
  // nswindow.contentView.wantsLayer = YES;

  // MTLClearColor color = MTLClearColorMake(0.7, 0.1, 0.2, 1.0);

  // // set all our state properties
  // state->device = gpu;
  // state->cmd_queue = queue;
  // state->swapchain = swapchain;
  // state->clear_color = color;

  // NSError *err = 0x0; // TEMPORARY

  // WARN("About to try loading metallib");
  // id<MTLLibrary> defaultLibrary = [state->device newLibraryWithFile: @"build/gfx.metallib" error:&err];
  // CASSERT(defaultLibrary);
  // state->default_lib = defaultLibrary;
  // if (!state->default_lib) {
  //     NSLog(@"Failed to load library");
  //     exit(0);
  // }

  // create_render_pipeline(state);

  return true;
}

void gpu_backend_shutdown() {}

bool gpu_device_create(gpu_device* out_device) {
  TRACE("GPU Device creation");
  const id<MTLDevice> gpu = MTLCreateSystemDefaultDevice();
  out_device->id = gpu;
  context.device = out_device;

  const id<MTLCommandQueue> queue = [gpu newCommandQueue];
  context.command_queue = queue;

  return true;
}
void gpu_device_destroy() {}

// --- Render Pipeline
gpu_pipeline* gpu_graphics_pipeline_create(struct graphics_pipeline_desc description) {
  TRACE("GPU Graphics Pipeline creation");
  // Allocate
  // gpu_pipeline_layout* layout =
  //     pipeline_layout_pool_alloc(&context.gpu_pools.pipeline_layouts, NULL);
  gpu_pipeline* pipeline = pipeline_pool_alloc(&context.gpu_pools.pipelines, NULL);

  WARN("About to try loading metallib");
  assert(description.vs.is_combined_vert_frag);
  // Ignore fragment shader data, as vert shader data contains both
  NSError *err = 0x0; // TEMPORARY
  NSString *myNSString = [NSString stringWithUTF8String:(char*)description.vs.filepath.buf];
  id<MTLLibrary> default_library = [context.device->id newLibraryWithFile:myNSString error:&err];
  assert(default_library);

  // setup vertex and fragment shaders
  id<MTLFunction> ren_vert = [default_library newFunctionWithName:@"basic_vertex"];
  assert(ren_vert);
  id<MTLFunction> ren_frag = [default_library newFunctionWithName:@"basic_fragment"];
  assert(ren_frag);

  // create pipeline descriptor
  @autoreleasepool {
    NSError *err = 0x0;
    MTLRenderPipelineDescriptor *pld = [[MTLRenderPipelineDescriptor alloc] init];
    NSString *pipeline_name = [NSString stringWithUTF8String: description.debug_name];
    pld.label = pipeline_name;
    pld.vertexFunction = ren_vert;
    pld.fragmentFunction = ren_frag;
    pld.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pld.colorAttachments[0].blendingEnabled = YES;

    MTLDepthStencilDescriptor *depthStencilDescriptor = [MTLDepthStencilDescriptor new];
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
    depthStencilDescriptor.depthWriteEnabled = YES;
    pld.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;

    id<MTLDepthStencilState> depth_descriptor = [context.device->id newDepthStencilStateWithDescriptor:depthStencilDescriptor];
    // FIXME: state->depth_state = depth_descriptor;

    id<MTLRenderPipelineState> pipeline_state = [context.device->id newRenderPipelineStateWithDescriptor:pld error:&err];
    TRACE("created renderpipelinestate");
    pipeline->pipeline_state = pipeline_state;

  }

  return pipeline;
}
void gpu_pipeline_destroy(gpu_pipeline* pipeline) {}

// --- Renderpass
gpu_renderpass* gpu_renderpass_create(const gpu_renderpass_desc* description) {
  gpu_renderpass* renderpass = renderpass_pool_alloc(&context.gpu_pools.renderpasses, NULL);

  // TODO: Configure based on description
   // set up render pass
  context.surface = [context.swapchain->swapchain nextDrawable];
  MTLRenderPassDescriptor *renderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
  MTLRenderPassColorAttachmentDescriptor *cd = renderPassDescriptor.colorAttachments[0];
  [cd setTexture:context.surface.texture];
  [cd setLoadAction:MTLLoadActionClear];
  MTLClearColor clearColor = MTLClearColorMake(0.1, 0.1, 0.0, 1.0);
  [cd setClearColor:clearColor];
  [cd setStoreAction:MTLStoreActionStore];

  renderpass->rpass_descriptor = renderPassDescriptor;

  return renderpass;
}

void gpu_renderpass_destroy(gpu_renderpass* pass) {}

// --- Swapchain
bool gpu_swapchain_create(gpu_swapchain* out_swapchain) {
  TRACE("GPU Swapchain creation");
  CAMetalLayer *swapchain = [CAMetalLayer layer];
  swapchain.device = context.device->id;
  swapchain.opaque = YES;
  out_swapchain->swapchain = swapchain;

  // set swapchain for the window
  context.metal_window.contentView.layer = swapchain;
  context.metal_window.contentView.wantsLayer = YES;

  context.swapchain = out_swapchain;
  return true;
}
void gpu_swapchain_destroy(gpu_swapchain* swapchain) {}

// --- Command buffer
gpu_cmd_encoder gpu_cmd_encoder_create() {
  id <MTLCommandBuffer> cmd_buffer = [context.command_queue commandBuffer];
  
  return (gpu_cmd_encoder) {
    .cmd_buffer = cmd_buffer
  };
}
void gpu_cmd_encoder_destroy(gpu_cmd_encoder* encoder) {}
void gpu_cmd_encoder_begin(gpu_cmd_encoder encoder) { /* no-op */ }
void gpu_cmd_encoder_begin_render(gpu_cmd_encoder* encoder, gpu_renderpass* renderpass) {
  DEBUG("Create Render Command Encoder");
  id<MTLRenderCommandEncoder> render_encoder = [encoder->cmd_buffer renderCommandEncoderWithDescriptor:renderpass->rpass_descriptor];
  encoder->render_encoder = render_encoder;
  // [encoder setDepthStencilState:state->depth_state];
}
void gpu_cmd_encoder_end_render(gpu_cmd_encoder* encoder) {}
void gpu_cmd_encoder_begin_compute() {}
gpu_cmd_encoder* gpu_get_default_cmd_encoder() {
  return &context.main_command_buf;
}

/** @brief Finish recording and return a command buffer that can be submitted to a queue */
gpu_cmd_buffer gpu_cmd_encoder_finish(gpu_cmd_encoder* encoder) {}

void gpu_queue_submit(gpu_cmd_buffer* buffer) {}

void encode_buffer_copy(gpu_cmd_encoder* encoder, buffer_handle src, u64 src_offset,
                        buffer_handle dst, u64 dst_offset, u64 copy_size);
void buffer_upload_bytes(buffer_handle gpu_buf, bytebuffer cpu_buf, u64 offset, u64 size);

void copy_buffer_to_buffer_oneshot(buffer_handle src, u64 src_offset, buffer_handle dst,
                                   u64 dst_offset, u64 copy_size);
void copy_buffer_to_image_oneshot(buffer_handle src, texture_handle dst);

void encode_bind_pipeline(gpu_cmd_encoder* encoder, pipeline_kind kind, gpu_pipeline* pipeline) {}
void encode_bind_shader_data(gpu_cmd_encoder* encoder, u32 group, shader_data* data) {}
void encode_set_default_settings(gpu_cmd_encoder* encoder) {
  [encoder->render_encoder setCullMode:MTLCullModeBack];
}
void encode_set_vertex_buffer(gpu_cmd_encoder* encoder, buffer_handle buf) {
  gpu_buffer* vertex_buf = BUFFER_GET(buf);
  [encoder->render_encoder setVertexBuffer:vertex_buf->id offset:0 atIndex:0];
}
void encode_set_index_buffer(gpu_cmd_encoder* encoder, buffer_handle buf) {}
void encode_set_bind_group() {}
void encode_draw(gpu_cmd_encoder* encoder) {}
void encode_draw_indexed(gpu_cmd_encoder* encoder, u64 index_count) {}
void encode_clear_buffer(gpu_cmd_encoder* encoder, buffer_handle buf) {}

buffer_handle gpu_buffer_create(u64 size, gpu_buffer_type buf_type, gpu_buffer_flags flags,
                                const void* data) {
  buffer_handle handle;
  gpu_buffer* buffer = buffer_pool_alloc(&context.resource_pools->buffers, &handle);
  buffer->size = size;

  id<MTLBuffer> mtl_vert_buf = [context.device->id newBufferWithBytes:data
      length: size
      options:MTLResourceStorageModeShared];
  return handle;
}
void gpu_buffer_destroy(buffer_handle buffer) {}
void gpu_buffer_upload(const void* data) {}

texture_handle gpu_texture_create(texture_desc desc, bool create_view, const void* data) {}
void gpu_texture_destroy(texture_handle) {}
void gpu_texture_upload(texture_handle texture, const void* data) {}

bool gpu_backend_begin_frame() {
  context.main_command_buf.cmd_buffer = [context.command_queue commandBuffer];
  return true;
  }
void gpu_backend_end_frame() {}
void gpu_temp_draw(size_t n_verts) {}

#endif