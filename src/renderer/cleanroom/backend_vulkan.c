#include <stdlib.h>
#include "ral.h"
#include "types.h"
#include "render_types.h"

#define VULKAN_QUEUES_COUNT 2
const char* queue_names[VULKAN_QUEUES_COUNT] = {
  "GRAPHICS", "TRANSFER"
};

typedef struct vulkan_context {
  gpu_device device;
  
  VkInstance instance;

} vulkan_context;

static vulkan_context context;

static bool select_physical_device(gpu_device* out_device) {}

bool gpu_device_create(gpu_device* out_device) {
  // Physical device
  if (!select_physical_device(out_device)) {
    return false;
  }
  INFO("Physical device selected");

  // Logical device
  VkDeviceQueueCreateInfo queue_create_info[2];
  //..
  VkDeviceCreateInfo device_create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

  VkResult result = vkCreateDevice();
  if (result != VK_SUCCESS) {
    FATAL("Error creating logical device with status %u\n", result);
    exit(1);
  }
  INFO("Logical device created");

  // Queues

  // Create the command pool

}

gpu_renderpass* gpu_renderpass_create() {
  // Allocate it
  // sets everything up
  // return pointer to it
}

void encode_set_pipeline(gpu_cmd_encoder* encoder, pipeline_type kind, gpu_pipeline* pipeline) {
//                        VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
  if (kind== PIPELINE_GRAPHICS) {
    // ...
  } else {
    // ...
  }
}

// --- Drawing
inline void encode_draw_indexed(gpu_cmd_encoder* encoder, u64 index_count) {
  vkCmdDrawIndexed(encoder->cmd_buffer, index_count, 1, 0, 0, 0);
}