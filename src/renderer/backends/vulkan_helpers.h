#pragma once

#include <assert.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "darray.h"
#include "defines.h"
#include "log.h"
#include "str.h"

#define VULKAN_PHYS_DEVICE_MAX_EXTENSION_NAMES 36

DECL_TYPED_ARRAY(const char*, cstr)

static void plat_get_required_extension_names(cstr_darray* extensions) {
#ifdef CEL_PLATFORM_LINUX
  cstr_darray_push(extensions, "VK_KHR_xcb_surface");
#endif
}

// TODO(omni): port to using internal assert functions
#define VK_CHECK(vulkan_expr)              \
  do {                                     \
    VkResult res = vulkan_expr;            \
    if (res != VK_SUCCESS) {               \
      ERROR_EXIT("Vulkan error: %u (%s:%d)", res, __FILE__, __LINE__); \
    }                                      \
  } while (0)

// TODO: typedef struct vk_debugger {} vk_debugger;

typedef struct vulkan_physical_device_requirements {
  bool graphics;
  bool present;
  bool compute;
  bool transfer;
  str8 device_ext_names[VULKAN_PHYS_DEVICE_MAX_EXTENSION_NAMES];
  size_t device_ext_name_count;
  bool sampler_anistropy;
  bool discrete_gpu;
} vulkan_physical_device_requirements;

#define VULKAN_MAX_DEFAULT 32

typedef struct vulkan_swapchain_support_info {
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR formats[VULKAN_MAX_DEFAULT];
  u32 format_count;
  VkPresentModeKHR present_modes[VULKAN_MAX_DEFAULT];
  u32 mode_count;
} vulkan_swapchain_support_info;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT flags,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);

static void vulkan_device_query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface,
                                                  vulkan_swapchain_support_info* out_support_info) {
  // TODO: add VK_CHECK to these calls!

  // Surface capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &out_support_info->capabilities);

  // Surface formats
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &out_support_info->format_count,
                                       0);  // Get number of formats
  if (out_support_info->format_count > 0) {
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &out_support_info->format_count,
                                         out_support_info->formats);
  }

  // Present Modes
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &out_support_info->mode_count,
                                            0);  // Get number of formats
  if (out_support_info->mode_count > 0) {
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &out_support_info->mode_count,
                                              out_support_info->present_modes);
  }
}

static VkSurfaceFormatKHR choose_swapchain_format(
    vulkan_swapchain_support_info* swapchain_support) {
  assert(swapchain_support->format_count > 0);
  // find a format
  for (u32 i = 0; i < swapchain_support->format_count; i++) {
    VkSurfaceFormatKHR format = swapchain_support->formats[i];
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  return swapchain_support->formats[0];
}

// static bool physical_device_meets_requirements(
//     VkPhysicalDevice device, VkSurfaceKHR surface, const VkPhysicalDeviceProperties* properties,
//     const VkPhysicalDeviceFeatures* features,
//     const vulkan_physical_device_requirements* requirements,
//     vulkan_physical_device_queue_family_info* out_queue_info,
//     vulkan_swapchain_support_info* out_swapchain_support) {
//   // TODO: pass in an arena

//   out_queue_info->graphics_family_index = -1;
//   out_queue_info->present_family_index = -1;
//   out_queue_info->compute_family_index = -1;
//   out_queue_info->transfer_family_index = -1;

//   if (requirements->discrete_gpu) {
//     if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
//       TRACE("Device is not a physical GPU. Skipping.");
//       return false;
//     }
//   }

//   u32 queue_family_count = 0;
//   vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
//   VkQueueFamilyProperties queue_families[queue_family_count];
//   vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

//   INFO("Graphics | Present | Compute | Transfer | Name");
//   u8 min_transfer_score = 255;
//   for (u32 i = 0; i < queue_family_count; i++) {
//     u8 current_transfer_score = 0;

//     // Graphics queue
//     if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//       out_queue_info->graphics_family_index = i;
//       current_transfer_score++;
//     }

//     // Compute queue
//     if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
//       out_queue_info->compute_family_index = i;
//       current_transfer_score++;
//     }

//     // Transfer queue
//     if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
//       // always take the lowest score transfer index
//       if (current_transfer_score <= min_transfer_score) {
//         min_transfer_score = current_transfer_score;
//         out_queue_info->transfer_family_index = i;
//       }
//     }

//     // Present Queue
//     VkBool32 supports_present = VK_FALSE;
//     vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present);
//     if (supports_present) {
//       out_queue_info->present_family_index = i;
//     }
//   }

//   INFO("       %d |       %d |       %d |        %d | %s",
//        out_queue_info->graphics_family_index != -1, out_queue_info->present_family_index != -1,
//        out_queue_info->compute_family_index != -1, out_queue_info->transfer_family_index != -1,
//        properties->deviceName);
//   TRACE("Graphics Family queue index: %d", out_queue_info->graphics_family_index);
//   TRACE("Present Family queue index: %d", out_queue_info->present_family_index);
//   TRACE("Compute Family queue index: %d", out_queue_info->compute_family_index);
//   TRACE("Transfer Family queue index: %d", out_queue_info->transfer_family_index);

//   if ((!requirements->graphics ||
//        (requirements->graphics && out_queue_info->graphics_family_index != -1))) {
//     INFO("Physical device meets our requirements! Proceed.");

//     vulkan_device_query_swapchain_support(
//         device, surface, out_swapchain_support

//         // TODO: error handling i.e. format count = 0 or present mode = 0

//     );
//     return true;
//   }

//   return false;
// }

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT flags,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
  switch (severity) {
    default:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      ERROR("%s", callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      WARN("%s", callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      INFO("%s", callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      TRACE("%s", callback_data->pMessage);
      break;
  }
  return VK_FALSE;
}