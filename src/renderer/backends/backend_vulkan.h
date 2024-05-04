#pragma once
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "defines.h"
#include "ral.h"
// #include "vulkan_helpers.h"

#define GPU_SWAPCHAIN_IMG_COUNT 2

/*
Conventions:
  - Place the 'handle' as the first field of a struct
  - Vulkan specific data goes at the top, followed by our internal data
*/

typedef struct queue_family_indices {
  u32 graphics_family_index;
  u32 present_family_index;
  u32 compute_family_index;
  u32 transfer_family_index;
  bool has_graphics;
  bool has_present;
  bool has_compute;
  bool has_transfer;
} queue_family_indices;

typedef struct gpu_swapchain {
  VkSwapchainKHR handle;
  arena swapchain_arena;
  VkSurfaceFormatKHR image_format;
  VkPresentModeKHR present_mode;
  VkImage* images;
  u32 image_count;
} gpu_swapchain;

typedef struct gpu_device {
  // In Vulkan we store both physical and logical device here
  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
  queue_family_indices queue_family_indicies;
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue compute_queue;
  VkQueue transfer_queue;
  VkCommandPool pool;
} gpu_device;

typedef struct gpu_pipeline_layout {
  VkPipelineLayout handle;
} gpu_pipeline_layout;

typedef struct gpu_pipeline {
  VkPipeline handle;
  VkPipelineLayout layout_handle;
} gpu_pipeline;

typedef struct gpu_renderpass {
  VkRenderPass handle;
  VkFramebuffer framebuffers[GPU_SWAPCHAIN_IMG_COUNT];
} gpu_renderpass;

typedef struct gpu_cmd_encoder {
  VkCommandBuffer cmd_buffer;
} gpu_cmd_encoder;