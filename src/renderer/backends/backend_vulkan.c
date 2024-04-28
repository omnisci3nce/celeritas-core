#include <glfw3.h>
#include <stdlib.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "backend_vulkan.h"
#include "mem.h"
#include "vulkan_helpers.h"

#include "defines.h"
#include "log.h"
#include "ral.h"
#include "ral_types.h"

// TEMP
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000

#define VULKAN_QUEUES_COUNT 2
const char* queue_names[VULKAN_QUEUES_COUNT] = { "GRAPHICS", "TRANSFER" };

typedef struct vulkan_context {
  VkInstance instance;
  VkAllocationCallbacks* allocator;
  VkSurfaceKHR surface;

  arena temp_arena;
  gpu_device* device;
  gpu_swapchain* swapchain;

  u32 screen_width;
  u32 screen_height;
} vulkan_context;

static vulkan_context context;

// --- Function forward declarations

/** @brief Enumerates and selects the most appropriate graphics device */
bool select_physical_device(gpu_device* out_device);
/** @brief Helper function for creating array of all extensions we want */
cstr_darray* get_all_extensions();

bool gpu_backend_init(const char* window_name, GLFWwindow* window) {
  context.allocator = 0;  // TODO: use an allocator
  context.screen_width = SCREEN_WIDTH;
  context.screen_height = SCREEN_HEIGHT;

  // Create an allocator
  size_t temp_arena_size = 1024 * 1024;
  arena_create(malloc(temp_arena_size), temp_arena_size);

  // Setup Vulkan instance
  VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
  app_info.apiVersion = VK_API_VERSION_1_3;
  app_info.pApplicationName = window_name;
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Celeritas Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
  create_info.pApplicationInfo = &app_info;

  // Extensions
  // FIXME: Use my own extension choices
  // cstr_darray* required_extensions = cstr_darray_new(2);
  // cstr_darray_push(required_extensions, VK_KHR_SURFACE_EXTENSION_NAME);
  // create_info.enabledExtensionCount = cstr_darray_len(required_extensions);
  // create_info.ppEnabledExtensionNames = required_extensions->data;
  uint32_t count;
  const char** extensions = glfwGetRequiredInstanceExtensions(&count);
  create_info.enabledExtensionCount = count;
  create_info.ppEnabledExtensionNames = extensions;

  // TODO: Validation layers
  create_info.enabledLayerCount = 0;
  create_info.ppEnabledLayerNames = NULL;

  VkResult result = vkCreateInstance(&create_info, NULL, &context.instance);
  if (result != VK_SUCCESS) {
    ERROR("vkCreateInstance failed with result: %u", result);
    return false;
  }
  TRACE("Vulkan Instance created");

  // Surface creation
  VkSurfaceKHR surface;
  VK_CHECK(glfwCreateWindowSurface(context.instance, window, NULL, &surface));
  context.surface = surface;
  TRACE("Vulkan Surface created");

  return true;
}

void gpu_backend_shutdown() { arena_free_storage(&context.temp_arena); }

bool gpu_device_create(gpu_device* out_device) {
  // Physical device
  if (!select_physical_device(out_device)) {
    return false;
  }
  TRACE("Physical device selected");

  // Features
  VkPhysicalDeviceFeatures device_features = { 0 };
  device_features.samplerAnisotropy = VK_TRUE;  // request anistrophy

  // Logical device
  VkDeviceQueueCreateInfo queue_create_info[2];
  //..
  VkDeviceCreateInfo device_create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
  device_create_info.queueCreateInfoCount = VULKAN_QUEUES_COUNT;
  device_create_info.pQueueCreateInfos = queue_create_info;
  device_create_info.pEnabledFeatures = &device_features;
  device_create_info.enabledExtensionCount = 1;
  const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  device_create_info.ppEnabledExtensionNames = &extension_names;

  VkResult result = vkCreateDevice(out_device->physical_device, &device_create_info,
                                   context.allocator, &out_device->logical_device);
  if (result != VK_SUCCESS) {
    FATAL("Error creating logical device with status %u\n", result);
    exit(1);
  }
  TRACE("Logical device created");

  // Queues

  // Create the command pool

  return true;
}

bool gpu_swapchain_create(gpu_swapchain* out_swapchain) {
  VkExtent2D swapchain_extent = { context.screen_width, context.screen_height };

  // find a format

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;  // guaranteed to be implemented

  VkSwapchainCreateInfoKHR swapchain_create_info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };

  // swapchain_create_info.minImageCount =

  VK_CHECK(vkCreateSwapchainKHR(context.device->logical_device, &swapchain_create_info,
                                context.allocator, &out_swapchain->handle));
  TRACE("Vulkan Swapchain created");
}

gpu_pipeline* gpu_graphics_pipeline_create(struct graphics_pipeline_desc description) {
  // Allocate
  gpu_pipeline_layout* layout = malloc(sizeof(gpu_pipeline_layout));
  gpu_pipeline* pipeline = malloc(sizeof(gpu_pipeline));

  // Viewport
  VkViewport viewport = { .x = 0,
                          .y = 0,
                          .width = (f32)context.screen_width,
                          .height = (f32)context.screen_height,
                          .minDepth = 0.0,
                          .maxDepth = 1.0 };
  VkRect2D scissor = { .offset = { .x = 0, .y = 0 },
                       .extent = { .width = context.screen_width,
                                   .height = context.screen_height } };
  VkPipelineViewportStateCreateInfo viewport_state = {
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
  };
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
  };
  rasterizer_create_info.depthClampEnable = VK_FALSE;
  rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
  rasterizer_create_info.polygonMode =
      description.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
  rasterizer_create_info.lineWidth = 1.0f;
  rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer_create_info.depthBiasEnable = VK_FALSE;
  rasterizer_create_info.depthBiasConstantFactor = 0.0;
  rasterizer_create_info.depthBiasClamp = 0.0;
  rasterizer_create_info.depthBiasSlopeFactor = 0.0;

  // Multisampling
  VkPipelineMultisampleStateCreateInfo ms_create_info = {
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
  };
  ms_create_info.sampleShadingEnable = VK_FALSE;
  ms_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  ms_create_info.minSampleShading = 1.0;
  ms_create_info.pSampleMask = 0;
  ms_create_info.alphaToCoverageEnable = VK_FALSE;
  ms_create_info.alphaToOneEnable = VK_FALSE;

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil = {
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
  };
  depth_stencil.depthTestEnable = description.depth_test ? VK_TRUE : VK_FALSE;
  depth_stencil.depthWriteEnable = description.depth_test ? VK_TRUE : VK_FALSE;
  depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depth_stencil.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.stencilTestEnable = VK_FALSE;
  depth_stencil.pNext = 0;

  // TODO: Blending

  // TODO: Vertex Input

  // TODO: Attributes

  // TODO: layouts
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
  };
  pipeline_layout_create_info.setLayoutCount = 0;
  pipeline_layout_create_info.pSetLayouts = NULL;
  pipeline_layout_create_info.pushConstantRangeCount = 0;
  pipeline_layout_create_info.pPushConstantRanges = NULL;
  VK_CHECK(vkCreatePipelineLayout(context.device->logical_device, &pipeline_layout_create_info,
                                  context.allocator, &layout->handle));
  pipeline->layout_handle = layout->handle;  // keep a copy of the layout on the pipeline object

  VkGraphicsPipelineCreateInfo pipeline_create_info = {
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
  };
  // pipeline_create_info.stageCount = stage_count;
  // pipeline_create_info.pStages = stages;
  // pipeline_create_info.pVertexInputState = &vertex_input_info;
  // pipeline_create_info.pInputAssemblyState = &input_assembly;

  // pipeline_create_info.pViewportState = &viewport_state;
  // pipeline_create_info.pRasterizationState = &rasterizer_create_info;
  // pipeline_create_info.pMultisampleState = &ms_create_info;
  // pipeline_create_info.pDepthStencilState = &depth_stencil;
  // pipeline_create_info.pColorBlendState = &color_blend;
  // pipeline_create_info.pDynamicState = &dynamic_state;
  // pipeline_create_info.pTessellationState = 0;

  // pipeline_create_info.layout = out_pipeline->layout;

  // pipeline_create_info.renderPass = renderpass->handle;
  // pipeline_create_info.subpass = 0;
  // pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
  // pipeline_create_info.basePipelineIndex = -1;

  VkResult result =
      vkCreateGraphicsPipelines(context.device->logical_device, VK_NULL_HANDLE, 1,
                                &pipeline_create_info, context.allocator, &pipeline->handle);
  if (result != VK_SUCCESS) {
    FATAL("graphics pipeline creation failed. its fked mate");
    ERROR_EXIT("Doomed");
  }

  return pipeline;
}

gpu_renderpass* gpu_renderpass_create(const gpu_renderpass_desc* description) {
  // TEMP: allocate with malloc. in the future we will have a pool allocator on the context
  gpu_renderpass* renderpass = malloc(sizeof(gpu_renderpass));

  // Colour attachment
  VkAttachmentDescription color_attachment;
  // color_attachment.format = context->swapchain.image_format.format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  color_attachment.flags = 0;

  // attachment_descriptions[0] = color_attachment;

  VkAttachmentReference color_attachment_reference;
  color_attachment_reference.attachment = 0;
  color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // subpass.colorAttachmentCount = 1;
  // subpass.pColorAttachments = &color_attachment_reference;

  // TODO: Depth attachment

  // main subpass
  VkSubpassDescription subpass = { 0 };
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_reference;

  // sets everything up

  // Finally, create the RenderPass
  VkRenderPassCreateInfo render_pass_create_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

  return renderpass;
}

void encode_set_pipeline(gpu_cmd_encoder* encoder, gpu_pipeline* pipeline) {
  //                        VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
  // if (kind == PIPELINE_GRAPHICS) {
  //   // ...
  // } else {
  //   // ...
  // }
}

// --- Drawing
inline void encode_draw_indexed(gpu_cmd_encoder* encoder, u64 index_count) {
  vkCmdDrawIndexed(encoder->cmd_buffer, index_count, 1, 0, 0, 0);
}

bool select_physical_device(gpu_device* out_device) {}