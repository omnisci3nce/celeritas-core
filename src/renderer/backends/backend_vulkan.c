#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "backend_vulkan.h"

#include "defines.h"
#include "log.h"
#include "ral.h"
#include "ral_types.h"

#define VULKAN_QUEUES_COUNT 2
const char* queue_names[VULKAN_QUEUES_COUNT] = { "GRAPHICS", "TRANSFER" };

typedef struct vulkan_context {
  gpu_device device;
  VkAllocationCallbacks* allocator;

  VkInstance instance;

} vulkan_context;

static vulkan_context context;

static bool select_physical_device(gpu_device* out_device) {}

gpu_device gpu_device_create() {
  gpu_device device = { 0 };
  // Physical device
  // if (!select_physical_device()) {
  //   return false;
  // }
  INFO("Physical device selected");

  // Features
  VkPhysicalDeviceFeatures device_features = {};
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

  VkResult result = vkCreateDevice(device.physical_device, &device_create_info, context.allocator,
                                   &device.logical_device);
  if (result != VK_SUCCESS) {
    FATAL("Error creating logical device with status %u\n", result);
    exit(1);
  }
  INFO("Logical device created");

  // Queues

  // Create the command pool

  context.device = device;
  return device;
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