#pragma once

#ifdef CEL_REND_BACKEND_VULKAN
#include "ral_impl.h"
#include "defines.h"
#include "maths_types.h"
#include "ral.h"
#include "ral_types.h"

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// Provide definitions for RAL structs

struct GPU_Swapchain {
    VkSwapchainKHR handle;
};

struct GPU_Device {
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
};

struct GPU_PipelineLayout {};
 struct GPU_Pipeline {};
 struct GPU_Renderpass {};
 struct GPU_CmdEncoder {};
 struct GPU_CmdBuffer {};
 struct GPU_Buffer {
     VkBuffer handle;
     VkDeviceMemory memory;
     u64 size;
 };
 struct GPU_Texture {
     VkImage handle;
     VkDeviceMemory memory;
     u64 size;
     VkImageView view;
     VkSampler sampler;
     char* debug_label;
 };

#endif
