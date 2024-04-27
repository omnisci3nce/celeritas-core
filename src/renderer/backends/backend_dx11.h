#pragma once
#include "ral.h"

#define GPU_SWAPCHAIN_IMG_COUNT 2

typedef struct gpu_swapchain {
} gpu_swapchain;
typedef struct gpu_device {
  // VkPhysicalDevice physical_device;
  // VkDevice logical_device;
  // VkPhysicalDeviceProperties properties;
  // VkPhysicalDeviceFeatures features;
  // VkPhysicalDeviceMemoryProperties memory;
  // VkCommandPool pool;
} gpu_device;
typedef struct gpu_pipeline {
} gpu_pipeline;

typedef struct gpu_renderpass {
  // VkRenderPass vk_handle;
  // VkFramebuffer framebuffers[GPU_SWAPCHAIN_IMG_COUNT];
  // u32
} gpu_renderpass;

typedef struct gpu_cmd_encoder {
  // VkCommandBuffer cmd_buffer;
} gpu_cmd_encoder;