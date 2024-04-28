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
  VkViewport viewport = { .x = 0,
                          .y = 0,
                          .width = (f32)context.screen_width,
                          .height = (f32)context.screen_height,
                          .minDepth = 0.0,
                          .maxDepth = 1.0 };
  VkRect2D scissor = { .offset = { .x = 0, .y = 0 },
                       .extent = { .width = context.screen_width,
                                   .height = context.screen_height } };

  // TODO: Attributes

  // TODO: layouts

  VkPipelineViewportStateCreateInfo viewport_state = {
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
  };
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;
}

gpu_renderpass* gpu_renderpass_create() {
  // Allocate it
  // sets everything up
  // return pointer to it
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