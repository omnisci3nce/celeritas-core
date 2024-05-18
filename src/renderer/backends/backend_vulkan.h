#pragma once
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "defines.h"
#include "mem.h"
#include "ral.h"
#include "ral_types.h"

#define MAX_FRAMES_IN_FLIGHT 2
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

// typedef struct vulkan_framebuffer {
// } vulkan_framebuffer;

typedef struct gpu_swapchain {
  VkSwapchainKHR handle;
  arena swapchain_arena;
  VkExtent2D extent;
  VkSurfaceFormatKHR image_format;
  VkPresentModeKHR present_mode;
  u32 image_count;
  VkImage* images;
  VkImageView* image_views;
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

typedef struct desc_set_uniform_buffer {
  VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
  VkDeviceMemory uniform_buf_memorys[MAX_FRAMES_IN_FLIGHT];
  void* uniform_buf_mem_mappings[MAX_FRAMES_IN_FLIGHT];
  size_t size;
} desc_set_uniform_buffer;

typedef struct gpu_pipeline {
  VkPipeline handle;
  VkPipelineLayout layout_handle;

  // Descriptor gubbins
  shader_data data_layouts[MAX_SHADER_DATA_LAYOUTS];
  u32 data_layouts_count;

  VkDescriptorSetLayout* desc_set_layouts;
  // Based on group, we know which data to load
  desc_set_uniform_buffer* uniform_pointers;
  u32 desc_set_layouts_count;

} gpu_pipeline;

typedef struct gpu_renderpass {
  VkRenderPass handle;
  // TODO: Where to store framebuffers? VkFramebuffer framebuffers[GPU_SWAPCHAIN_IMG_COUNT];
} gpu_renderpass;

typedef struct gpu_cmd_encoder {
  VkCommandBuffer cmd_buffer;
  VkDescriptorPool descriptor_pool;
  gpu_pipeline* pipeline;
} gpu_cmd_encoder;

typedef struct gpu_cmd_buffer {
  VkCommandBuffer cmd_buffer;
} gpu_cmd_buffer;

typedef struct gpu_buffer {
  VkBuffer handle;
  VkDeviceMemory memory;
  u64 size;
} gpu_buffer;

typedef struct gpu_texture {
  VkImage handle;
  VkDeviceMemory memory;
  u64 size;
  texture_desc desc;
} gpu_texture;
