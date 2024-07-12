/**
 * @brief
*/
#pragma once
#include "defines.h"
#include "ral_types.h"

struct GLFWwindow;

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

bool GPU_Backend_Init(const char* window_name, struct GLFWwindow* window);
void GPU_Backend_Shutdown();

bool GPU_Device_Create(GPU_Device* out_device);
void GPU_Device_Destroy(GPU_Device* device);

bool GPU_Swapchain_Create(GPU_Swapchain* out_swapchain);
void GPU_Swapchain_Destroy(GPU_Swapchain* swapchain);

GPU_Renderpass* GPU_Renderpass_Create(GPU_RenderpassDesc description);
void GPU_Renderpass_Destroy(GPU_Renderpass* pass);

GPU_Pipeline* GPU_GraphicsPipeline_Create(GraphicsPipelineDesc description, GPU_Renderpass* renderpass);
void GraphicsPipeline_Destroy(GPU_Pipeline* pipeline);

#if defined(CEL_REND_BACKEND_OPENGL)
#include "backend_opengl.h"
#endif
