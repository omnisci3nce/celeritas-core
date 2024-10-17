#include <celeritas.h>

#ifdef GPU_METAL

#define MTL_DEBUG_LAYER 1 // enable all metal validation layers

// Obj-C imports
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>
#include <CoreGraphics/CGGeometry.h>

#define GLFW_INCLUDE_NONE
#import <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#import <GLFW/glfw3native.h>
#include "stb_image.h"

NAMESPACED_LOGGER(metal);

// --- RAL types

struct gpu_swapchain {
  int width, height;
  CAMetalLayer* swapchain;
};

struct gpu_encoder {
  id<MTLCommandBuffer> cmd_buffer;
  id<MTLRenderCommandEncoder> cmd_encoder;
};

typedef struct metal_pipeline {
  id<MTLRenderPipelineState> pso;
} metal_pipeline;

typedef struct metal_buffer {
  id<MTLBuffer> id;
} metal_buffer;

typedef struct metal_texture {
  id<MTLTexture> id;
} metal_texture;

TYPED_POOL(metal_buffer, buf);
TYPED_POOL(metal_texture, tex);
TYPED_POOL(metal_pipeline, pipeline);

typedef struct metal_context {
  GLFWwindow* window;
  NSWindow* metal_window;

  id<MTLDevice> device;
  id<CAMetalDrawable> surface;
  gpu_swapchain default_swapchain;
  id<MTLLibrary> default_library;

  id<MTLCommandQueue> command_queue;

  /* pools */
  buf_pool bufpool;
  tex_pool texpool;
  pipeline_pool psopool; // pso = pipeline state object
} metal_context;

static metal_context ctx;

void ral_backend_init(const char* window_name, struct GLFWwindow* window) {
  TRACE("loading Metal backend");

  TRACE("gpu device creation");
  const id<MTLDevice> gpu = MTLCreateSystemDefaultDevice();
  ctx.device = gpu;

  TRACE("window init");
  glfwMakeContextCurrent(window);
  NSWindow* nswindow = glfwGetCocoaWindow(window);
  ctx.metal_window = nswindow;

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  // effectively the "framebuffer"
  CAMetalLayer* metal_layer = [CAMetalLayer layer];
  metal_layer.device = gpu;
  metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  metal_layer.drawableSize = CGSizeMake(width, height);
  ctx.metal_window.contentView.layer = metal_layer;
  ctx.metal_window.contentView.wantsLayer = true;
  ctx.default_swapchain.swapchain = metal_layer;

  TRACE("command queue creation");
  const id<MTLCommandQueue> queue = [ctx.device newCommandQueue];
  ctx.command_queue = queue;

  TRACE("resource pool init");
  metal_buffer* buffer_storage = malloc(sizeof(metal_buffer) * 100);
  ctx.bufpool = buf_pool_create(buffer_storage, 100, sizeof(metal_buffer));

  metal_texture* texture_storage = malloc(sizeof(metal_texture) * 100);
  ctx.texpool = tex_pool_create(texture_storage, 100, sizeof(metal_texture));

  metal_pipeline* pipeline_storage = malloc(sizeof(metal_pipeline) * 100);
  ctx.psopool = pipeline_pool_create(pipeline_storage, 100, sizeof(metal_pipeline));

  TRACE("create default metal lib");
  NSError* nserr = 0x0;
  id<MTLLibrary> default_library = [ctx.device newLibraryWithFile:@"build/shaders/default.metallib" error:&nserr];
  if (!default_library) {
    ERROR("Error loading metal lib\n");
    exit(1);
  }
  ctx.default_library = default_library;

  INFO("Successfully initialised Metal RAL backend");
}

void ral_backend_shutdown() {
  // no-op
}

buf_handle ral_buffer_create(u64 size, const void *data) {
  buf_handle handle;
  metal_buffer* buffer = buf_pool_alloc(&ctx.bufpool, &handle);
  buffer->id = [ctx.device newBufferWithBytes:data length:size options:MTLResourceStorageModeShared];

  return handle;
}

tex_handle ral_texture_create(texture_desc desc, bool create_view, const void *data) {
  tex_handle handle;
  metal_texture* texture = tex_pool_alloc(&ctx.texpool, &handle);

  MTLTextureDescriptor* texture_descriptor = [[MTLTextureDescriptor alloc] init];
  [texture_descriptor setPixelFormat:MTLPixelFormatRGBA8Unorm];
  [texture_descriptor setWidth:desc.width];
  [texture_descriptor setHeight:desc.height];

  texture->id = [ctx.device newTextureWithDescriptor:texture_descriptor];

  MTLRegion region = MTLRegionMake2D(0, 0, desc.width, desc.height);
  u32 bytes_per_row = 4 * desc.width;

  [texture->id replaceRegion:region mipmapLevel:0  withBytes:data bytesPerRow:bytes_per_row];

  [texture_descriptor release];

  return handle;
}

tex_handle ral_texture_load_from_file(const char* filepath) {
  texture_desc desc;

  stbi_set_flip_vertically_on_load(true);
  unsigned char* image = stbi_load(filepath, &desc.width, &desc.height, &desc.num_channels, STBI_rgb_alpha);
  assert(image != NULL);

  tex_handle handle = ral_texture_create(desc, false, image);
  stbi_image_free(image);
  return handle;
}

pipeline_handle ral_gfx_pipeline_create(gfx_pipeline_desc desc) {
  TRACE("creating graphics pipeline");

  pipeline_handle handle;
  metal_pipeline* p = pipeline_pool_alloc(&ctx.psopool, &handle);

  @autoreleasepool {
    // setup vertex and fragment shaders
    NSString* vertex_entry_point = [NSString stringWithUTF8String:desc.vertex.entry_point];
    id<MTLFunction> vertex_func = [ctx.default_library newFunctionWithName:vertex_entry_point];
    assert(vertex_func);

    NSString* fragment_entry_point = [NSString stringWithUTF8String:desc.fragment.entry_point];
    id<MTLFunction> fragment_func = [ctx.default_library newFunctionWithName:fragment_entry_point];
    assert(fragment_func);

    NSError* err = 0x0;
    MTLRenderPipelineDescriptor* pld = [[MTLRenderPipelineDescriptor alloc] init];
    // in auto release pool so dont need to call release()

    [pld setLabel:@"Pipeline"];
    [pld setVertexFunction:vertex_func];
    [pld setFragmentFunction:fragment_func];
    pld.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pld.colorAttachments[0].blendingEnabled = YES;
    assert(pld);

    id<MTLRenderPipelineState> pso = [ctx.device newRenderPipelineStateWithDescriptor:pld error:&err];
    assert(pso);
    p->pso = pso;
  }

  return handle;
}

gpu_encoder* ral_render_encoder(render_pass_desc rpass_desc) {
  id<MTLCommandBuffer> buffer = [ctx.command_queue commandBuffer];

  // create renderpass descriptor
  MTLRenderPassDescriptor* rpd = [[MTLRenderPassDescriptor alloc] init];
  MTLRenderPassColorAttachmentDescriptor* cd = rpd.colorAttachments[0];
  [cd setTexture:ctx.surface.texture];
  [cd setLoadAction:MTLLoadActionClear];
  MTLClearColor clearColor = MTLClearColorMake(41.0f/255.0f, 42.0f/255.0f, 48.0f/255.0f, 1.0);
  [cd setClearColor:clearColor];
  [cd setStoreAction:MTLStoreActionStore];

  id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:rpd];

  gpu_encoder* enc = malloc(sizeof(gpu_encoder));
  enc->cmd_buffer = buffer;
  enc->cmd_encoder = encoder;

  return enc;
}

void ral_encoder_finish_and_submit(gpu_encoder* enc) {
  [enc->cmd_encoder  endEncoding];
  [enc->cmd_buffer presentDrawable:ctx.surface];
  [enc->cmd_buffer commit];
  [enc->cmd_buffer waitUntilCompleted];
}

void ral_encode_bind_pipeline(gpu_encoder *enc, pipeline_handle pipeline) {
  metal_pipeline* p = pipeline_pool_get(&ctx.psopool, pipeline);
  [enc->cmd_encoder setRenderPipelineState:p->pso];
}

void ral_encode_set_vertex_buf(gpu_encoder *enc, buf_handle vbuf) {
  metal_buffer* b = buf_pool_get(&ctx.bufpool, vbuf);
  [enc->cmd_encoder setVertexBuffer:b->id offset:0 atIndex:0 ];
}

void ral_encode_set_texture(gpu_encoder* enc, tex_handle texture, u32 slot) {
  metal_texture* t = tex_pool_get(&ctx.texpool, texture);
  [enc->cmd_encoder setFragmentTexture:t->id atIndex:slot];
}

void ral_encode_draw_tris(gpu_encoder* enc, size_t start, size_t count) {
  MTLPrimitiveType tri_primitive = MTLPrimitiveTypeTriangle;
  [enc->cmd_encoder drawPrimitives:tri_primitive vertexStart:start vertexCount:count];
}

void ral_frame_start() {}

void ral_frame_draw(scoped_draw_commands draw_fn) {
  @autoreleasepool {
    ctx.surface = [ctx.default_swapchain.swapchain nextDrawable];
    draw_fn();
  }
}

void ral_frame_end() {}

void ral_backend_resize_framebuffer(int width, int height) {
  TRACE("resizing framebuffer");
  ctx.default_swapchain.swapchain.drawableSize = CGSizeMake((float)width, (float)height);
}

#endif