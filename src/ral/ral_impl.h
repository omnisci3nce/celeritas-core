/**
 * @brief
 */
#pragma once
#include "buf.h"
#include "defines.h"
#include "ral_types.h"

struct GLFWwindow;
struct ResourcePools;

// Forward declare structs - these must be defined in the backend implementation
typedef struct GPU_Swapchain GPU_Swapchain;
typedef struct GPU_Device GPU_Device;
typedef struct GPU_PipelineLayout GPU_PipelineLayout;
typedef struct GPU_Pipeline GPU_Pipeline;
typedef struct GPU_Renderpass GPU_Renderpass;
typedef struct GPU_CmdEncoder GPU_CmdEncoder;  // Recording
typedef struct GPU_CmdBuffer GPU_CmdBuffer;    // Ready for submission
typedef struct GPU_Buffer GPU_Buffer;
typedef struct GPU_Texture GPU_Texture;

bool GPU_Backend_Init(const char* window_name, struct GLFWwindow* window,
                      struct ResourcePools* res_pools);
void GPU_Backend_Shutdown();

bool GPU_Device_Create(GPU_Device* out_device);
void GPU_Device_Destroy(GPU_Device* device);

bool GPU_Swapchain_Create(GPU_Swapchain* out_swapchain);
void GPU_Swapchain_Destroy(GPU_Swapchain* swapchain);
void GPU_Swapchain_Resize(i32 new_width, i32 new_height);
u32x2 GPU_Swapchain_GetDimensions();

PUB GPU_Renderpass* GPU_Renderpass_Create(GPU_RenderpassDesc description);
PUB void GPU_Renderpass_Destroy(GPU_Renderpass* pass);

PUB GPU_Pipeline* GPU_GraphicsPipeline_Create(GraphicsPipelineDesc description,
                                              GPU_Renderpass* renderpass);
PUB void GraphicsPipeline_Destroy(GPU_Pipeline* pipeline);

// --- Command buffer
PUB GPU_CmdEncoder GPU_CmdEncoder_Create();
PUB void GPU_CmdEncoder_Destroy(GPU_CmdEncoder* encoder);
PUB void GPU_CmdEncoder_Begin(GPU_CmdEncoder* encoder);
PUB void GPU_CmdEncoder_Finish(GPU_CmdEncoder* encoder);
PUB void GPU_CmdEncoder_BeginRender(GPU_CmdEncoder* encoder, GPU_Renderpass* renderpass);
PUB void GPU_CmdEncoder_EndRender(GPU_CmdEncoder* encoder);
PUB GPU_CmdEncoder* GPU_GetDefaultEncoder();
PUB void GPU_QueueSubmit(GPU_CmdBuffer* cmd_buffer);

// --- Buffers
PUB BufferHandle GPU_BufferCreate(u64 size, GPU_BufferType buf_type, GPU_BufferFlags flags,
                                  const void* data);
PUB void GPU_BufferDestroy(BufferHandle handle);
PUB void GPU_BufferUpload(BufferHandle buffer, size_t n_bytes, const void* data);

// --- Textures
PUB TextureHandle GPU_TextureCreate(TextureDesc desc, bool create_view, const void* data);
PUB GPU_Texture* GPU_TextureAlloc(TextureHandle* out_handle);
PUB void GPU_TextureDestroy(TextureHandle handle);
PUB void GPU_TextureUpload(TextureHandle handle, size_t n_bytes, const void* data);

// --- Data copy commands
// TODO: Rename these to reflect current coding style
void encode_buffer_copy(GPU_CmdEncoder* encoder, BufferHandle src, u64 src_offset, BufferHandle dst,
                        u64 dst_offset, u64 copy_size);
void buffer_upload_bytes(BufferHandle gpu_buf, bytebuffer cpu_buf, u64 offset, u64 size);

void copy_buffer_to_buffer_oneshot(BufferHandle src, u64 src_offset, BufferHandle dst,
                                   u64 dst_offset, u64 copy_size);
void copy_buffer_to_image_oneshot(BufferHandle src, TextureHandle dst);

// --- Render commands
PUB void GPU_EncodeBindPipeline(GPU_CmdEncoder* encoder, GPU_Pipeline* pipeline);
PUB void GPU_EncodeBindShaderData(GPU_CmdEncoder* encoder, u32 group, ShaderDataLayout layout);
void GPU_EncodeSetDefaults(GPU_CmdEncoder* encoder);
PUB void GPU_EncodeSetVertexBuffer(GPU_CmdEncoder* encoder, BufferHandle buf);
PUB void GPU_EncodeSetIndexBuffer(GPU_CmdEncoder* encoder, BufferHandle buf);
PUB void GPU_EncodeCopyBufToBuf();

// PUB void GPU_EncodeCopyBufToTex(GPU_CmdEncoder* encoder, BufferHandle src, TextureHandle dst,
//     u32 x_offset, u32 y_offset, u32 width, u32 height, const void* data);
/** @brief Convenience method for writing data directly into a texture. Staging memory is handled
 * internally. */
PUB void GPU_WriteTextureRegion(GPU_CmdEncoder* encoder, TextureHandle dst, u32 x_offset,
                                u32 y_offset, u32 width, u32 height, const void* data);
PUB void GPU_WriteBuffer(GPU_CmdEncoder* encoder, BufferHandle buf, u64 offset, u64 size,
                         const void* data);

PUB void GPU_EncodeDraw(GPU_CmdEncoder* encoder, u64 count);
PUB void GPU_EncodeDrawIndexed(GPU_CmdEncoder* encoder, u64 index_count);
PUB void GPU_EncodeDrawInstanced(GPU_CmdEncoder* encoder, u64 index_count,
                                 u64 instance_count);  // TODO: implement instanced rendering

// --- Frame cycle
PUB bool GPU_Backend_BeginFrame();
PUB void GPU_Backend_EndFrame();

// Concrete implementation
#if defined(CEL_REND_BACKEND_OPENGL)
#include "backend_opengl.h"
#elif defined(CEL_REND_BACKEND_VULKAN)
#include "backend_vulkan.h"
#endif
