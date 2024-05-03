#include <assert.h>
#include <glfw3.h>
#include <stdlib.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "backend_vulkan.h"
#include "maths_types.h"
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

  VkDebugUtilsMessengerEXT vk_debugger;
} vulkan_context;

static vulkan_context context;

// --- Function forward declarations

/** @brief Enumerates and selects the most appropriate graphics device */
bool select_physical_device(gpu_device* out_device);

bool is_physical_device_suitable(VkPhysicalDevice device);

queue_family_indices find_queue_families(VkPhysicalDevice device);

bool create_logical_device(gpu_device* out_device);

/** @brief Helper function for creating array of all extensions we want */
cstr_darray* get_all_extensions();

bool gpu_backend_init(const char* window_name, GLFWwindow* window) {
  context.allocator = 0;  // TODO: use an allocator
  context.screen_width = SCREEN_WIDTH;
  context.screen_height = SCREEN_HEIGHT;

  // Create an allocator
  size_t temp_arena_size = 1024 * 1024;
  context.temp_arena = arena_create(malloc(temp_arena_size), temp_arena_size);

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
  cstr_darray* required_extensions = cstr_darray_new(2);
  // cstr_darray_push(required_extensions, VK_KHR_SURFACE_EXTENSION_NAME);

  uint32_t count;
  const char** extensions = glfwGetRequiredInstanceExtensions(&count);
  for (u32 i = 0; i < count; i++) {
    cstr_darray_push(required_extensions, extensions[i]);
  }

  cstr_darray_push(required_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  DEBUG("Required extensions:");
  for (u32 i = 0; i < cstr_darray_len(required_extensions); i++) {
    DEBUG("  %s", required_extensions->data[i]);
  }

  create_info.enabledExtensionCount = cstr_darray_len(required_extensions);
  create_info.ppEnabledExtensionNames = required_extensions->data;

  // TODO: Validation layers
  create_info.enabledLayerCount = 0;
  create_info.ppEnabledLayerNames = NULL;

  INFO("Validation layers enabled");
  cstr_darray* desired_validation_layers = cstr_darray_new(1);
  cstr_darray_push(desired_validation_layers, "VK_LAYER_KHRONOS_validation");

  u32 n_available_layers = 0;
  VK_CHECK(vkEnumerateInstanceLayerProperties(&n_available_layers, 0));
  TRACE("%d available layers", n_available_layers);
  VkLayerProperties* available_layers =
      arena_alloc(&context.temp_arena, n_available_layers * sizeof(VkLayerProperties));
  VK_CHECK(vkEnumerateInstanceLayerProperties(&n_available_layers, available_layers));

  for (int i = 0; i < cstr_darray_len(desired_validation_layers); i++) {
    // look through layers to make sure we can find the ones we want
    bool found = false;
    for (int j = 0; j < n_available_layers; j++) {
      if (str8_equals(str8_cstr_view(desired_validation_layers->data[i]),
                      str8_cstr_view(available_layers[j].layerName))) {
        found = true;
        TRACE("Found layer %s", desired_validation_layers->data[i]);
        break;
      }
    }

    if (!found) {
      FATAL("Required validation is missing %s", desired_validation_layers->data[i]);
      return false;
    }
  }
  INFO("All validation layers are present");
  create_info.enabledLayerCount = cstr_darray_len(desired_validation_layers);
  create_info.ppEnabledLayerNames = desired_validation_layers->data;

  VkResult result = vkCreateInstance(&create_info, NULL, &context.instance);
  if (result != VK_SUCCESS) {
    ERROR("vkCreateInstance failed with result: %u", result);
    return false;
  }
  TRACE("Vulkan Instance created");

  DEBUG("Creating Vulkan debugger");
  u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
    VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
  };
  debug_create_info.messageSeverity = log_severity;
  debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debug_create_info.pfnUserCallback = vk_debug_callback;

  PFN_vkCreateDebugUtilsMessengerEXT func =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance,
                                                                "vkCreateDebugUtilsMessengerEXT");
  assert(func);
  VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.vk_debugger));
  DEBUG("Vulkan Debugger created");

  // Surface creation
  VkSurfaceKHR surface;
  VK_CHECK(glfwCreateWindowSurface(context.instance, window, NULL, &surface));
  context.surface = surface;
  TRACE("Vulkan Surface created");

  return true;
}

void gpu_backend_shutdown() {
  arena_free_storage(&context.temp_arena);
  vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
  vkDestroyInstance(context.instance, context.allocator);
}

bool gpu_device_create(gpu_device* out_device) {
  // First things first store this poitner from the renderer
  context.device = out_device;

  arena_save savept = arena_savepoint(&context.temp_arena);
  // Physical device
  if (!select_physical_device(out_device)) {
    return false;
  }
  TRACE("Physical device selected");

  // Logical device
  create_logical_device(out_device);
  // VkDeviceQueueCreateInfo queue_create_info = {};

  // queue_family_indices indices = find_queue_families(context.device->physical_device);
  // //..
  // VkDeviceCreateInfo device_create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
  // device_create_info.queueCreateInfoCount = VULKAN_QUEUES_COUNT;
  // device_create_info.pQueueCreateInfos = queue_create_info;
  // device_create_info.pEnabledFeatures = &device_features;
  // device_create_info.enabledExtensionCount = 1;
  // const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  // device_create_info.ppEnabledExtensionNames = &extension_names;

  // VkResult result = vkCreateDevice(out_device->physical_device, &device_create_info,
  //                                  context.allocator, &out_device->logical_device);
  // if (result != VK_SUCCESS) {
  //   FATAL("Error creating logical device with status %u\n", result);
  //   exit(1);
  // }
  // TRACE("Logical device created");

  // Queues

  // Create the command pool

  arena_rewind(savept);  // Free any temp data
  return true;
}

bool gpu_swapchain_create(gpu_swapchain* out_swapchain) {
  VkExtent2D swapchain_extent = { context.screen_width, context.screen_height };

  // find a format

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;  // guaranteed to be implemented

  VkSwapchainCreateInfoKHR swapchain_create_info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  swapchain_create_info.surface = context.surface;
  swapchain_create_info.minImageCount = 2;
  // TODO: image_ fields
  swapchain_create_info.queueFamilyIndexCount = 0;
  swapchain_create_info.pQueueFamilyIndices = 0;

  // TODO: preTransform
  swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_create_info.presentMode = present_mode;

  swapchain_create_info.clipped = VK_TRUE;
  swapchain_create_info.oldSwapchain = 0;

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

bool select_physical_device(gpu_device* out_device) {
  u32 physical_device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(context.instance, &physical_device_count, 0));
  if (physical_device_count == 0) {
    FATAL("No devices that support vulkan were found");
    return false;
  }
  TRACE("Number of devices found %d", physical_device_count);

  VkPhysicalDevice* physical_devices =
      arena_alloc(&context.temp_arena, physical_device_count * sizeof(VkPhysicalDevice));
  VK_CHECK(vkEnumeratePhysicalDevices(context.instance, &physical_device_count, physical_devices));

  bool found = false;
  for (u32 device_i = 0; device_i < physical_device_count; device_i++) {
    if (is_physical_device_suitable(physical_devices[device_i])) {
      out_device->physical_device = physical_devices[device_i];
      found = true;
      break;
    }
  }

  if (!found) {
    FATAL("Couldn't find a suitable physical device");
    return false;
  }

  return true;
}

bool is_physical_device_suitable(VkPhysicalDevice device) {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);

  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(device, &features);

  VkPhysicalDeviceMemoryProperties memory;
  vkGetPhysicalDeviceMemoryProperties(device, &memory);

  // TODO: Check against these device properties

  queue_family_indices indices = find_queue_families(device);

  return indices.has_graphics && indices.has_present;
}

queue_family_indices find_queue_families(VkPhysicalDevice device) {
  queue_family_indices indices = { 0 };

  u32 queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);

  VkQueueFamilyProperties* queue_families =
      arena_alloc(&context.temp_arena, queue_family_count * sizeof(VkQueueFamilyProperties));
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

  for (u32 q_fam_i = 0; q_fam_i < queue_family_count; q_fam_i++) {
    // Graphics queue
    if (queue_families[q_fam_i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family_index = q_fam_i;
      indices.has_graphics = true;
    }

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, q_fam_i, context.surface, &present_support);
    if (present_support) {
      indices.present_family_index = q_fam_i;
      indices.has_present = true;
    }
  }

  return indices;
}

bool create_logical_device(gpu_device* out_device) {
  queue_family_indices indices = find_queue_families(out_device->physical_device);

  // Queues
  f32 prio_one = 1.0;
  VkDeviceQueueCreateInfo queue_create_infos[2] = { 0 };
  queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_infos[0].queueFamilyIndex = indices.graphics_family_index;
  queue_create_infos[0].queueCount = 1;
  queue_create_infos[0].pQueuePriorities = &prio_one;
  queue_create_infos[0].flags = 0;
  queue_create_infos[0].pNext = 0;

  queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_infos[1].queueFamilyIndex = indices.present_family_index;
  queue_create_infos[1].queueCount = 1;
  queue_create_infos[1].pQueuePriorities = &prio_one;
  queue_create_infos[1].flags = 0;
  queue_create_infos[1].pNext = 0;

  // Features
  VkPhysicalDeviceFeatures device_features = { 0 };
  device_features.samplerAnisotropy = VK_TRUE;  // request anistrophy

  // Device itself
  VkDeviceCreateInfo device_create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
  device_create_info.queueCreateInfoCount = 2;
  device_create_info.pQueueCreateInfos = queue_create_infos;
  device_create_info.pEnabledFeatures = &device_features;
  device_create_info.enabledExtensionCount = 1;
  const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  device_create_info.ppEnabledExtensionNames = &extension_names;

  // deprecated
  device_create_info.enabledLayerCount = 0;
  device_create_info.ppEnabledLayerNames = 0;

  VkResult result = vkCreateDevice(context.device->physical_device, &device_create_info,
                                   context.allocator, &context.device->logical_device);
  if (result != VK_SUCCESS) {
    printf("error creating logical device with status %u\n", result);
    ERROR_EXIT("Unable to create vulkan logical device. Exiting..");
  }
  TRACE("Logical device created");

  context.device->queue_family_indicies = indices;

  // Retrieve queue handles
  vkGetDeviceQueue(context.device->logical_device, indices.graphics_family_index, 0,
                   &context.device->graphics_queue);
  vkGetDeviceQueue(context.device->logical_device, indices.present_family_index, 0,
                   &context.device->present_queue);

  return true;
}