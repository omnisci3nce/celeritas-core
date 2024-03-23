#include <stdbool.h>
#define CEL_PLATFORM_LINUX
#include <assert.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "str.h"

#include "darray.h"
#include "defines.h"
#include "file.h"
#include "log.h"
#include "maths_types.h"
#include "render_backend.h"
#include "render_types.h"
#include "vulkan_helpers.h"

#include <stdlib.h>

#if CEL_REND_BACKEND_VULKAN

#include <glad/glad.h>

#include <glfw3.h>

typedef struct vulkan_device {
  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  vulkan_swapchain_support_info swapchain_support;
  i32 graphics_queue_index;
  i32 present_queue_index;
  i32 compute_queue_index;
  i32 transfer_queue_index;
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue compute_queue;
  VkQueue transfer_queue;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
} vulkan_device;

typedef struct vulkan_swapchain {
  VkSurfaceFormatKHR image_format;
  u8 max_frames_in_flight;
  VkSwapchainKHR handle;
  u32 image_count;
  VkImage* images;
  VkImageView* views;
} vulkan_swapchain;

typedef struct vulkan_context {
  VkInstance instance;
  VkAllocationCallbacks* allocator;
  VkSurfaceKHR surface;
  vulkan_device device;
  vulkan_swapchain swapchain;
  u32 image_index;
  u32 current_frame;

  // TODO: swapchain recreation

#if defined(DEBUG)
  VkDebugUtilsMessengerEXT vk_debugger;
#endif
} vulkan_context;

static vulkan_context context;

/** @brief Internal backend state */
typedef struct vulkan_state {
} vulkan_state;

KITC_DECL_TYPED_ARRAY(VkLayerProperties)

bool select_physical_device(vulkan_context* ctx) {
  u32 physical_device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, 0));
  if (physical_device_count == 0) {
    FATAL("No devices that support vulkan were found");
    return false;
  }
  TRACE("Number of devices found %d", physical_device_count);

  VkPhysicalDevice physical_devices[physical_device_count];
  VK_CHECK(vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, physical_devices));

  for (u32 i = 0; i < physical_device_count; i++) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);

    vulkan_physical_device_requirements requirements = {};
    requirements.graphics = true;
    requirements.present = true;
    requirements.compute = true;
    requirements.transfer = true;

    requirements.sampler_anistropy = true;
    requirements.discrete_gpu = true;
    requirements.device_ext_names[0] = str8lit(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    requirements.device_ext_name_count = 1;

    vulkan_physical_device_queue_family_info queue_info = {};

    bool result = physical_device_meets_requirements(physical_devices[i], ctx->surface, &properties,
                                                     &features, &requirements, &queue_info,
                                                     &ctx->device.swapchain_support);

    if (result) {
      INFO("GPU Driver version: %d.%d.%d", VK_VERSION_MAJOR(properties.driverVersion),
           VK_VERSION_MINOR(properties.driverVersion), VK_VERSION_PATCH(properties.driverVersion));

      INFO("Vulkan API version: %d.%d.%d", VK_VERSION_MAJOR(properties.apiVersion),
           VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));

      // TODO: print gpu memory information -
      // https://youtu.be/6Kj3O2Ov1RU?si=pXfP5NvXXcXjJsrG&t=2439

      ctx->device.physical_device = physical_devices[i];
      ctx->device.graphics_queue_index = queue_info.graphics_family_index;
      ctx->device.present_queue_index = queue_info.present_family_index;
      ctx->device.compute_queue_index = queue_info.compute_family_index;
      ctx->device.transfer_queue_index = queue_info.transfer_family_index;
      ctx->device.properties = properties;
      ctx->device.features = features;
      ctx->device.memory = memory;
      break;
    }
  }

  if (!ctx->device.physical_device) {
    ERROR("No suitable physical devices were found :(");
    return false;
  }

  INFO("Physical device selected: %s\n", ctx->device.properties.deviceName);
  return true;
}

bool vulkan_device_create(vulkan_context* context) {
  // Physical device - NOTE: mutates the context directly
  if (!select_physical_device(context)) {
    return false;
  }

// Logical device - NOTE: mutates the context directly

// queues
#define VULKAN_QUEUES_COUNT 3
  const char* queue_names[VULKAN_QUEUES_COUNT] = { "GRAPHICS", "PRESENT",
                                                   // "COMPUTE",
                                                   "TRANSFER" };
  i32 indices[VULKAN_QUEUES_COUNT] = { context->device.graphics_queue_index,
                                       context->device.present_queue_index,
                                       //  context->device.compute_queue_index,
                                       context->device.transfer_queue_index };
  VkDeviceQueueCreateInfo
      queue_create_info[VULKAN_QUEUES_COUNT];  // one for each of graphics,present,compute,transfer
  for (int i = 0; i < VULKAN_QUEUES_COUNT; i++) {
    TRACE("Configure %s queue", queue_names[i]);
    queue_create_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[i].queueFamilyIndex = indices[i];
    queue_create_info[i].queueCount = 1;  // make just one of them
    queue_create_info[i].flags = 0;
    queue_create_info[i].pNext = 0;
    f32 priority = 1.0;
    queue_create_info[i].pQueuePriorities = &priority;
  }

  // features
  VkPhysicalDeviceFeatures device_features = {};
  device_features.samplerAnisotropy = VK_TRUE;  // request anistrophy

  // device itself
  VkDeviceCreateInfo device_create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
  device_create_info.queueCreateInfoCount = VULKAN_QUEUES_COUNT;
  device_create_info.pQueueCreateInfos = queue_create_info;
  device_create_info.pEnabledFeatures = &device_features;
  device_create_info.enabledExtensionCount = 1;
  const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  device_create_info.ppEnabledExtensionNames = &extension_names;

  // deprecated
  device_create_info.enabledLayerCount = 0;
  device_create_info.ppEnabledLayerNames = 0;

  VkResult result = vkCreateDevice(context->device.physical_device, &device_create_info,
                                   context->allocator, &context->device.logical_device);
  if (result != VK_SUCCESS) {
    printf("error creating logical device with status %u\n", result);
    ERROR_EXIT("Bye bye");
  }
  INFO("Logical device created");

  // get queues
  vkGetDeviceQueue(context->device.logical_device, context->device.graphics_queue_index, 0,
                   &context->device.graphics_queue);
  vkGetDeviceQueue(context->device.logical_device, context->device.present_queue_index, 0,
                   &context->device.present_queue);
  vkGetDeviceQueue(context->device.logical_device, context->device.compute_queue_index, 0,
                   &context->device.compute_queue);
  vkGetDeviceQueue(context->device.logical_device, context->device.transfer_queue_index, 0,
                   &context->device.transfer_queue);

  return true;
}
void vulkan_device_destroy(vulkan_context* context) {
  context->device.physical_device = 0;  // release
  // TODO: reset other memory
}

void vulkan_swapchain_create(vulkan_context* context, u32 width, u32 height,
                             vulkan_swapchain* out_swapchain) {
  VkExtent2D swapchain_extent = { width, height };
  out_swapchain->max_frames_in_flight = 2;  // support triple buffering

  // find a format
  bool found;
  for (u32 i = 0; i < context->device.swapchain_support.format_count; i++) {
    VkSurfaceFormatKHR format = context->device.swapchain_support.formats[i];
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      out_swapchain->image_format = format;
      found = true;
      break;
    }
  }
  if (!found) {
    out_swapchain->image_format = context->device.swapchain_support.formats[0];
  }

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;  // guaranteed to be implemented
  // TODO: look for mailbox - https://youtu.be/jWKVb_QdSNM?si=bHcd3sEf-M0x3QwH&t=1687

  // TODO: requery swapchain support

  u32 image_count = context->device.swapchain_support.capabilities.minImageCount + 1;

  VkSwapchainCreateInfoKHR swapchain_create_info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  swapchain_create_info.surface = context->surface;
  swapchain_create_info.minImageCount = image_count;
  swapchain_create_info.imageFormat = out_swapchain->image_format.format;
  swapchain_create_info.imageColorSpace = out_swapchain->image_format.colorSpace;
  swapchain_create_info.imageExtent = swapchain_extent;
  swapchain_create_info.imageArrayLayers = 1;
  swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_create_info.queueFamilyIndexCount = 0;
  swapchain_create_info.pQueueFamilyIndices = 0;

  swapchain_create_info.preTransform =
      context->device.swapchain_support.capabilities.currentTransform;
  swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_create_info.presentMode = present_mode;
  swapchain_create_info.clipped = VK_TRUE;
  swapchain_create_info.oldSwapchain = 0;

  vkCreateSwapchainKHR(context->device.logical_device, &swapchain_create_info, context->allocator,
                       &out_swapchain->handle);

  context->current_frame = 0;

  // images
  out_swapchain->image_count = 0;
  vkGetSwapchainImagesKHR(context->device.logical_device, out_swapchain->handle,
                          &out_swapchain->image_count, 0);

  if (!out_swapchain->images) {
    out_swapchain->images = (VkImage*)malloc(sizeof(VkImage) * out_swapchain->image_count);
  }
  if (!out_swapchain->views) {
    out_swapchain->views = (VkImageView*)malloc(sizeof(VkImage) * out_swapchain->image_count);
  }
  VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device, out_swapchain->handle,
                                   &out_swapchain->image_count, out_swapchain->images));

  // views
  for (int i = 0; i < out_swapchain->image_count; i++) {
    VkImageViewCreateInfo view_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    view_info.image = out_swapchain->images[i];
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context->device.logical_device, &view_info, context->allocator,
                               &out_swapchain->views[i]));
  }
}

// TODO: swapchain destroy
void vulkan_swapchain_recreate(vulkan_context* context, u32 width, u32 height,
                               vulkan_swapchain* swapchain) {
  // TODO
}
bool vulkan_swapchain_acquire_next_image_index(vulkan_context* context, vulkan_swapchain* swapchain,
                                               u64 timeout_ns,
                                               VkSemaphore image_available_semaphore, VkFence fence,
                                               u32* out_image_index) {
  VkResult result =
      vkAcquireNextImageKHR(context->device.logical_device, swapchain->handle, timeout_ns,
                            image_available_semaphore, fence, out_image_index);

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    FATAL("Failed to acquire swapchain image");
    return false;
  }

  return true;
}

void vulkan_swapchain_present(vulkan_context* context, vulkan_swapchain* swapchain,
                              VkQueue graphics_queue, VkQueue present_queue,
                              VkSemaphore render_complete_semaphore, u32 present_image_index) {
  // return image to swapchain for presentation
  VkPresentInfoKHR present_info = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &render_complete_semaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain->handle;
  present_info.pImageIndices = &present_image_index;
  present_info.pResults = 0;

  VkResult result = vkQueuePresentKHR(present_queue, &present_info);
  if (result != VK_SUCCESS) {
    FATAL("Failed to present swapchain iamge");
  }
}

bool gfx_backend_init(renderer* ren) {
  INFO("loading Vulkan backend");

  vulkan_state* internal = malloc(sizeof(vulkan_state));
  ren->backend_state = (void*)internal;

  context.allocator = 0;  // TODO: custom allocator

  // Setup Vulkan instance
  VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
  app_info.apiVersion = VK_API_VERSION_1_3;
  app_info.pApplicationName = ren->config.window_name;
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Celeritas Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
  create_info.pApplicationInfo = &app_info;

  cstr_darray* required_extensions = cstr_darray_new(2);
  cstr_darray_push(required_extensions, VK_KHR_SURFACE_EXTENSION_NAME);

  plat_get_required_extension_names(required_extensions);

#if defined(DEBUG)
  cstr_darray_push(required_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  DEBUG("Required extensions:");
  for (u32 i = 0; i < cstr_darray_len(required_extensions); i++) {
    DEBUG("  %s", required_extensions->data[i]);
  }
#endif

  create_info.enabledExtensionCount = cstr_darray_len(required_extensions);
  create_info.ppEnabledExtensionNames = required_extensions->data;

  // Validation layers
  create_info.enabledLayerCount = 0;
  create_info.ppEnabledLayerNames = 0;
#if defined(DEBUG)
  INFO("Validation layers enabled");
  cstr_darray* desired_validation_layers = cstr_darray_new(1);
  cstr_darray_push(desired_validation_layers, "VK_LAYER_KHRONOS_validation");

  u32 n_available_layers = 0;
  VK_CHECK(vkEnumerateInstanceLayerProperties(&n_available_layers, 0));
  TRACE("%d available layers", n_available_layers);
  VkLayerProperties_darray* available_layers = VkLayerProperties_darray_new(n_available_layers);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&n_available_layers, available_layers->data));

  for (int i = 0; i < cstr_darray_len(desired_validation_layers); i++) {
    // look through layers to make sure we can find the ones we want
    bool found = false;
    for (int j = 0; j < n_available_layers; j++) {
      if (str8_equals(str8_cstr_view(desired_validation_layers->data[i]),
                      str8_cstr_view(available_layers->data[j].layerName))) {
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
#endif

  VkResult result = vkCreateInstance(&create_info, NULL, &context.instance);
  if (result != VK_SUCCESS) {
    ERROR("vkCreateInstance failed with result: %u", result);
    return false;
  }

  // Debugger
#if defined(DEBUG)
  DEBUG("Creating Vulkan debugger")
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
  DEBUG("Vulkan debugger created");

#endif

  // Surface creation
  DEBUG("Create SurfaceKHR")
  VkSurfaceKHR surface;
  VK_CHECK(glfwCreateWindowSurface(context.instance, ren->window, NULL, &surface));
  context.surface = surface;
  DEBUG("Vulkan surface created")

  // Device creation
  if (!vulkan_device_create(&context)) {
    FATAL("device creation failed");
    return false;
  }

  INFO("Vulkan renderer initialisation succeeded");
  return true;
}

void gfx_backend_shutdown(renderer* ren) {
  DEBUG("Destroying Vulkan debugger");
  if (context.vk_debugger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context.instance, "vkDestroyDebugUtilsMessengerEXT");
    func(context.instance, context.vk_debugger, context.allocator);
  }

  DEBUG("Destroying Vulkan instance...");
  vkDestroyInstance(context.instance, context.allocator);
}

void clear_screen(vec3 colour) {}

void bind_texture(shader s, texture* tex, u32 slot) {}
void bind_mesh_vertex_buffer(void* backend, mesh* mesh) {}
void draw_primitives(cel_primitive_topology primitive, u32 start_index, u32 count) {}

shader shader_create_separate(const char* vert_shader, const char* frag_shader) {}
void set_shader(shader s) {}

void uniform_vec3f(u32 program_id, const char* uniform_name, vec3* value) {}
void uniform_f32(u32 program_id, const char* uniform_name, f32 value) {}
void uniform_i32(u32 program_id, const char* uniform_name, i32 value) {}
void uniform_mat4f(u32 program_id, const char* uniform_name, mat4* value) {}

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

#endif