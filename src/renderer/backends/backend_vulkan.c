#include <assert.h>
#include <glfw3.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "backend_vulkan.h"
#include "buf.h"
#include "darray.h"
#include "maths_types.h"
#include "mem.h"
#include "ral_types.h"
#include "str.h"
#include "vulkan_helpers.h"

#include "defines.h"
#include "file.h"
#include "log.h"
#include "ral.h"
#include "utils.h"

// TEMP
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000
#define VULKAN_QUEUES_COUNT 2
#define MAX_DESCRIPTOR_SETS 10

const char* queue_names[VULKAN_QUEUES_COUNT] = { "GRAPHICS", "TRANSFER" };

KITC_DECL_TYPED_ARRAY(VkDescriptorSet)

typedef struct vulkan_context {
  VkInstance instance;
  VkAllocationCallbacks* allocator;
  VkSurfaceKHR surface;
  vulkan_swapchain_support_info swapchain_support;

  arena temp_arena;
  gpu_device* device;
  gpu_swapchain* swapchain;
  u32 framebuffer_count;
  VkFramebuffer*
      swapchain_framebuffers;  // TODO: Move this data into the swapchain as its own struct

  u32 current_img_index;
  u32 current_frame;  // super important
  gpu_cmd_encoder main_cmd_bufs[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
  VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];

  // HACK
  VkRenderPass main_renderpass;

  u32 screen_width;
  u32 screen_height;
  bool is_resizing;
  GLFWwindow* window;

  // Storage
  gpu_buffer buffers[1024];
  size_t buffer_count;
  VkDescriptorSet_darray* free_set_queue;
  struct resource_pools* resource_pools;

  VkDebugUtilsMessengerEXT vk_debugger;
} vulkan_context;

static vulkan_context context;

// --- Function forward declarations

/** @brief Enumerates and selects the most appropriate graphics device */
bool select_physical_device(gpu_device* out_device);

bool is_physical_device_suitable(VkPhysicalDevice device);

queue_family_indices find_queue_families(VkPhysicalDevice device);

bool create_logical_device(gpu_device* out_device);
void create_swapchain_framebuffers();
void create_sync_objects();
void create_descriptor_pools();
size_t vertex_attrib_size(vertex_attrib_type attr);

VkShaderModule create_shader_module(str8 spirv);

/** @brief Helper function for creating array of all extensions we want */
cstr_darray* get_all_extensions();

void vulkan_transition_image_layout(gpu_texture* texture, VkFormat format, VkImageLayout old_layout,
                                    VkImageLayout new_layout);

// --- Handy macros
#define BUFFER_GET(h) (buffer_pool_get(&context.resource_pools->buffers, h))
#define TEXTURE_GET(h) (texture_pool_get(&context.resource_pools->textures, h))

bool gpu_backend_init(const char* window_name, GLFWwindow* window) {
  memset(&context, 0, sizeof(vulkan_context));
  context.allocator = 0;  // TODO: use an allocator
  context.screen_width = SCREEN_WIDTH;
  context.screen_height = SCREEN_HEIGHT;
  context.window = window;
  context.current_img_index = 0;
  context.current_frame = 0;
  context.free_set_queue = VkDescriptorSet_darray_new(100);

  // Create an allocator
  size_t temp_arena_size = 1024 * 1024;
  context.temp_arena = arena_create(malloc(temp_arena_size), temp_arena_size);

  // Setup Vulkan instance
  VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
  app_info.apiVersion = VK_API_VERSION_1_2;
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
  gpu_swapchain_destroy(context.swapchain);

  vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
  vkDestroyInstance(context.instance, context.allocator);
  arena_free_storage(&context.temp_arena);
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

  // Logical device & Queues
  create_logical_device(out_device);

  // Create the command pool
  VkCommandPoolCreateInfo pool_create_info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
  pool_create_info.queueFamilyIndex = out_device->queue_family_indicies.graphics_family_index;
  pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  vkCreateCommandPool(out_device->logical_device, &pool_create_info, context.allocator,
                      &out_device->pool);
  TRACE("Command Pool created");

  // Synchronisation objects
  create_sync_objects();
  TRACE("Synchronisation primitives created");

  arena_rewind(savept);  // Free any temp data
  return true;
}

bool gpu_swapchain_create(gpu_swapchain* out_swapchain) {
  context.swapchain = out_swapchain;

  out_swapchain->swapchain_arena = arena_create(malloc(1024), 1024);

  vulkan_device_query_swapchain_support(context.device->physical_device, context.surface,
                                        &context.swapchain_support);
  vulkan_swapchain_support_info swapchain_support = context.swapchain_support;

  // TODO: custom swapchain extents VkExtent2D swapchain_extent = { width, height };

  VkSurfaceFormatKHR image_format = choose_swapchain_format(&swapchain_support);
  out_swapchain->image_format = image_format;
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;  // guaranteed to be implemented
  out_swapchain->present_mode = present_mode;

  u32 image_count = swapchain_support.capabilities.minImageCount + 1;
  out_swapchain->image_count = image_count;

  VkSwapchainCreateInfoKHR swapchain_create_info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  swapchain_create_info.surface = context.surface;
  swapchain_create_info.minImageCount = image_count;
  swapchain_create_info.imageFormat = image_format.format;
  swapchain_create_info.imageColorSpace = image_format.colorSpace;
  swapchain_create_info.imageExtent = swapchain_support.capabilities.currentExtent;
  swapchain_create_info.imageArrayLayers = 1;
  swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_create_info.queueFamilyIndexCount = 0;
  swapchain_create_info.pQueueFamilyIndices = NULL;

  swapchain_create_info.preTransform = swapchain_support.capabilities.currentTransform;
  swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_create_info.presentMode = present_mode;
  swapchain_create_info.clipped = VK_TRUE;
  swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

  out_swapchain->extent = swapchain_support.capabilities.currentExtent;

  VK_CHECK(vkCreateSwapchainKHR(context.device->logical_device, &swapchain_create_info,
                                context.allocator, &out_swapchain->handle));
  TRACE("Vulkan Swapchain created");

  // Retrieve Images
  // out_swapchain->images =
  //     arena_alloc(&out_swapchain->swapchain_arena, image_count * sizeof(VkImage));
  out_swapchain->images = malloc(image_count * sizeof(VkImage));
  VK_CHECK(vkGetSwapchainImagesKHR(context.device->logical_device, out_swapchain->handle,
                                   &image_count, out_swapchain->images));

  // Create ImageViews
  // TODO: Move this to a separate function
  out_swapchain->image_views = malloc(image_count * sizeof(VkImageView));
  // arena_alloc(&out_swapchain->swapchain_arena, image_count * sizeof(VkImageView));
  for (u32 i = 0; i < image_count; i++) {
    VkImageViewCreateInfo view_create_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    view_create_info.image = out_swapchain->images[i];
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = image_format.format;
    view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;
    vkCreateImageView(context.device->logical_device, &view_create_info, context.allocator,
                      &out_swapchain->image_views[i]);
  }

  return true;
}

void gpu_swapchain_destroy(gpu_swapchain* swapchain) {
  // Destroy Framebuffers
  DEBUG("Image count %d", swapchain->image_count);
  for (u32 i = 0; i < swapchain->image_count; i++) {
    DEBUG("Framebuffer handle %d", context.swapchain_framebuffers[i]);
    vkDestroyFramebuffer(context.device->logical_device, context.swapchain_framebuffers[i],
                         context.allocator);
  }
  for (u32 i = 0; i < swapchain->image_count; i++) {
    vkDestroyImageView(context.device->logical_device, swapchain->image_views[i],
                       context.allocator);
  }
  arena_free_all(&swapchain->swapchain_arena);
  vkDestroySwapchainKHR(context.device->logical_device, swapchain->handle, context.allocator);
  TRACE("Vulkan Swapchain destroyed");
}

static void recreate_swapchain(gpu_swapchain* swapchain) {
  int width = 0, height = 0;
  glfwGetFramebufferSize(context.window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(context.window, &width, &height);
    glfwWaitEvents();
  }
  DEBUG("Recreating swapchain...");
  vkDeviceWaitIdle(context.device->logical_device);

  gpu_swapchain_destroy(swapchain);
  gpu_swapchain_create(swapchain);
  create_swapchain_framebuffers();
}

VkFormat format_from_vertex_attr(vertex_attrib_type attr) {
  switch (attr) {
    case ATTR_F32:
      return VK_FORMAT_R32_SFLOAT;
    case ATTR_U32:
      return VK_FORMAT_R32_UINT;
    case ATTR_I32:
      return VK_FORMAT_R32_SINT;
    case ATTR_F32x2:
      return VK_FORMAT_R32G32_SFLOAT;
    case ATTR_U32x2:
      return VK_FORMAT_R32G32_UINT;
    case ATTR_I32x2:
      return VK_FORMAT_R32G32_UINT;
    case ATTR_F32x3:
      return VK_FORMAT_R32G32B32_SFLOAT;
    case ATTR_U32x3:
      return VK_FORMAT_R32G32B32_UINT;
    case ATTR_I32x3:
      return VK_FORMAT_R32G32B32_SINT;
    case ATTR_F32x4:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    case ATTR_U32x4:
      return VK_FORMAT_R32G32B32A32_UINT;
    case ATTR_I32x4:
      return VK_FORMAT_R32G32B32A32_SINT;
  }
}

gpu_pipeline* gpu_graphics_pipeline_create(struct graphics_pipeline_desc description) {
  // Allocate
  gpu_pipeline_layout* layout = malloc(sizeof(gpu_pipeline_layout));
  gpu_pipeline* pipeline = malloc(sizeof(gpu_pipeline));

  // Shaders
  printf("Vertex shader: %s\n", description.vs.filepath.buf);
  printf("Fragment shader: %s\n", description.fs.filepath.buf);
  VkShaderModule vertex_shader = create_shader_module(description.vs.code);
  VkShaderModule fragment_shader = create_shader_module(description.fs.code);

  // Vertex
  VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
  };
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_info.module = vertex_shader;
  vert_shader_stage_info.pName = "main";
  // Fragment
  VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
  };
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = fragment_shader;
  frag_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[2] = { vert_shader_stage_info,
                                                       frag_shader_stage_info };

  // Attributes
  u32 attr_count = description.vertex_desc.attributes_count;
  printf("N attributes %d\n", attr_count);
  VkVertexInputAttributeDescription attribute_descs[attr_count];
  memset(attribute_descs, 0, attr_count * sizeof(VkVertexInputAttributeDescription));
  u32 offset = 0;
  for (u32 i = 0; i < description.vertex_desc.attributes_count; i++) {
    attribute_descs[i].binding = 0;
    attribute_descs[i].location = i;
    attribute_descs[i].format = format_from_vertex_attr(description.vertex_desc.attributes[i]);
    attribute_descs[i].offset = offset;
    size_t this_offset = vertex_attrib_size(description.vertex_desc.attributes[i]);
    printf("offset total %d this attr %ld\n", offset, this_offset);
    printf("sizeof vertex %ld\n", sizeof(vertex));
    /* printf("%d \n", offsetof(vertex, static_3d)); */
    offset += this_offset;
  }
  /* printf("Vertex attributes\n"); */
  /* attribute_descs[0].binding = 0; */
  /* attribute_descs[0].location = 0; */
  /* attribute_descs[0].format = VK_FORMAT_R32G32B32_SFLOAT; */
  /* attribute_descs[0].offset = 0;  // offsetof(custom_vertex, pos); */

  /* attribute_descs[1].binding = 0; */
  /* attribute_descs[1].location = 1; */
  /* attribute_descs[1].format = VK_FORMAT_R32G32B32_SFLOAT; */
  /* attribute_descs[1].offset = 12;  // offsetof(custom_vertex, color); */

  // Vertex input
  // TODO: Generate this from descroiption now
  VkVertexInputBindingDescription binding_desc;
  binding_desc.binding = 0;
  binding_desc.stride = description.vertex_desc.use_full_vertex_size
                            ? sizeof(vertex)
                            : description.vertex_desc.stride;
  binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkPipelineVertexInputStateCreateInfo vertex_input_info = {
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
  };
  vertex_input_info.vertexBindingDescriptionCount = 1;
  vertex_input_info.pVertexBindingDescriptions = &binding_desc;
  vertex_input_info.vertexAttributeDescriptionCount =
      attr_count;  // description.vertex_desc.attributes_count;
  vertex_input_info.pVertexAttributeDescriptions = attribute_descs;

  // Input Assembly
  VkPipelineInputAssemblyStateCreateInfo input_assembly = {
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
  };
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  // Viewport
  VkViewport viewport = { .x = 0,
                          .y = 0,
                          .width = (f32)context.swapchain->extent.width,
                          .height = (f32)context.swapchain->extent.height,
                          .minDepth = 0.0,
                          .maxDepth = 1.0 };
  VkRect2D scissor = { .offset = { .x = 0, .y = 0 }, .extent = context.swapchain->extent };
  VkPipelineViewportStateCreateInfo viewport_state = {
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
  };
  viewport_state.viewportCount = 1;
  // viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  // viewport_state.pScissors = &scissor;

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
  /* rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE; */
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

  // TODO: Depth and stencil testing
  // VkPipelineDepthStencilStateCreateInfo depth_stencil = {
  //   VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
  // };
  // depth_stencil.depthTestEnable = description.depth_test ? VK_TRUE : VK_FALSE;
  // depth_stencil.depthWriteEnable = description.depth_test ? VK_TRUE : VK_FALSE;
  // depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
  // depth_stencil.depthBoundsTestEnable = VK_FALSE;
  // depth_stencil.stencilTestEnable = VK_FALSE;
  // depth_stencil.pNext = 0;

  // Blending
  VkPipelineColorBlendAttachmentState color_blend_attachment_state;
  color_blend_attachment_state.blendEnable = VK_FALSE;
  color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                VK_COLOR_COMPONENT_G_BIT |
                                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo color_blend = {
    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
  };
  color_blend.logicOpEnable = VK_FALSE;
  color_blend.logicOp = VK_LOGIC_OP_COPY;
  color_blend.attachmentCount = 1;
  color_blend.pAttachments = &color_blend_attachment_state;

// Dynamic state
#define DYNAMIC_STATE_COUNT 2
  VkDynamicState dynamic_states[DYNAMIC_STATE_COUNT] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };

  VkPipelineDynamicStateCreateInfo dynamic_state = {
    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
  };
  dynamic_state.dynamicStateCount = DYNAMIC_STATE_COUNT;
  dynamic_state.pDynamicStates = dynamic_states;

  // Descriptor Set layouts

  VkDescriptorSetLayout* desc_set_layouts =
      malloc(description.data_layouts_count * sizeof(VkDescriptorSetLayout));
  pipeline->desc_set_layouts = desc_set_layouts;
  pipeline->desc_set_layouts_count = description.data_layouts_count;
  if (description.data_layouts_count > 0) {
    pipeline->uniform_pointers =
        malloc(description.data_layouts_count * sizeof(desc_set_uniform_buffer));
  } else {
    pipeline->uniform_pointers = NULL;
  }

  // assert(description.data_layouts_count == 1);
  printf("data layouts %d\n", description.data_layouts_count);
  for (u32 layout_i = 0; layout_i < description.data_layouts_count; layout_i++) {
    shader_data_layout sdl = description.data_layouts[layout_i].shader_data_get_layout(NULL);
    TRACE("Got shader data layout %d's bindings! . found %d", layout_i, sdl.bindings_count);

    VkDescriptorSetLayoutBinding desc_set_bindings[sdl.bindings_count];

    // Bindings
    assert(sdl.bindings_count == 2);
    for (u32 binding_j = 0; binding_j < sdl.bindings_count; binding_j++) {
      desc_set_bindings[binding_j].binding = binding_j;
      desc_set_bindings[binding_j].descriptorCount = 1;
      switch (sdl.bindings[binding_j].type) {
        case SHADER_BINDING_BUFFER:
        case SHADER_BINDING_BYTES:
          desc_set_bindings[binding_j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
          desc_set_bindings[binding_j].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // FIXME: dont hardcode

          u64 buffer_size = sdl.bindings[binding_j].data.bytes.size;
          VkDeviceSize uniform_buf_size = buffer_size;
          // TODO: Create backing buffer

          VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
          VkDeviceMemory uniform_buf_memorys[MAX_FRAMES_IN_FLIGHT];
          void* uniform_buf_mem_mappings[MAX_FRAMES_IN_FLIGHT];
          // void* s?
          for (size_t frame_i = 0; frame_i < MAX_FRAMES_IN_FLIGHT; frame_i++) {
            buffer_handle uniform_buf_handle =
                gpu_buffer_create(buffer_size, CEL_BUFFER_UNIFORM, CEL_BUFFER_FLAG_CPU, NULL);

            gpu_buffer* created_gpu_buffer =
                BUFFER_GET(uniform_buf_handle);  // context.buffers[uniform_buf_handle.raw];
            buffers[frame_i] = created_gpu_buffer->handle;
            uniform_buf_memorys[frame_i] = created_gpu_buffer->memory;
            vkMapMemory(context.device->logical_device, uniform_buf_memorys[frame_i], 0,
                        uniform_buf_size, 0, &uniform_buf_mem_mappings[frame_i]);
            // now we have a pointer in unifrom_buf_mem_mappings we can write to
          }

          desc_set_uniform_buffer uniform_data;
          memcpy(&uniform_data.buffers, &buffers, sizeof(buffers));
          memcpy(&uniform_data.uniform_buf_memorys, &uniform_buf_memorys,
                 sizeof(uniform_buf_memorys));
          memcpy(&uniform_data.uniform_buf_mem_mappings, &uniform_buf_mem_mappings,
                 sizeof(uniform_buf_mem_mappings));
          uniform_data.size = buffer_size;

          pipeline->uniform_pointers[binding_j] = uniform_data;

          break;
        case SHADER_BINDING_TEXTURE:
          desc_set_bindings[binding_j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
          desc_set_bindings[binding_j].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  // FIXME: dont hardcode
          desc_set_bindings[binding_j].pImmutableSamplers = NULL;

          break;
        default:
          ERROR_EXIT("Unimplemented binding type!! in backend_vulkan");
      }
      switch (sdl.bindings[binding_j].vis) {
        case VISIBILITY_VERTEX:
          desc_set_bindings[binding_j].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
          break;
        case VISIBILITY_FRAGMENT:
          desc_set_bindings[binding_j].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
          break;
        case VISIBILITY_COMPUTE:
          WARN("Compute is not implemented yet");
          break;
      }
    }

    VkDescriptorSetLayoutCreateInfo desc_set_layout_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
    };
    desc_set_layout_info.bindingCount = sdl.bindings_count;
    desc_set_layout_info.pBindings = desc_set_bindings;

    VK_CHECK(vkCreateDescriptorSetLayout(context.device->logical_device, &desc_set_layout_info,
                                         context.allocator, &desc_set_layouts[layout_i]));
  }
  printf("Descriptor set layouts\n");

  // Layout
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
  };
  pipeline_layout_create_info.setLayoutCount = description.data_layouts_count;
  pipeline_layout_create_info.pSetLayouts = desc_set_layouts;
  pipeline_layout_create_info.pushConstantRangeCount = 0;
  pipeline_layout_create_info.pPushConstantRanges = NULL;
  VK_CHECK(vkCreatePipelineLayout(context.device->logical_device, &pipeline_layout_create_info,
                                  context.allocator, &layout->handle));
  pipeline->layout_handle = layout->handle;  // keep a copy of the layout on the pipeline object

  VkGraphicsPipelineCreateInfo pipeline_create_info = {
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
  };

  pipeline_create_info.stageCount = 2;
  pipeline_create_info.pStages = shader_stages;
  pipeline_create_info.pVertexInputState = &vertex_input_info;
  pipeline_create_info.pInputAssemblyState = &input_assembly;

  pipeline_create_info.pViewportState = &viewport_state;
  pipeline_create_info.pRasterizationState = &rasterizer_create_info;
  pipeline_create_info.pMultisampleState = &ms_create_info;
  pipeline_create_info.pDepthStencilState = NULL;  // &depth_stencil;
  pipeline_create_info.pColorBlendState = &color_blend;
  pipeline_create_info.pDynamicState = &dynamic_state;
  pipeline_create_info.pTessellationState = 0;

  pipeline_create_info.layout = layout->handle;

  pipeline_create_info.renderPass = description.renderpass->handle;
  pipeline_create_info.subpass = 0;
  pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_create_info.basePipelineIndex = -1;

  printf("About to create graphics pipeline\n");

  VkResult result =
      vkCreateGraphicsPipelines(context.device->logical_device, VK_NULL_HANDLE, 1,
                                &pipeline_create_info, context.allocator, &pipeline->handle);
  if (result != VK_SUCCESS) {
    FATAL("graphics pipeline creation failed. its fked mate");
    ERROR_EXIT("Doomed");
  }
  TRACE("Vulkan Graphics pipeline created");

  // once the pipeline has been created we can destroy these
  vkDestroyShaderModule(context.device->logical_device, vertex_shader, context.allocator);
  vkDestroyShaderModule(context.device->logical_device, fragment_shader, context.allocator);

  // Framebuffers
  create_swapchain_framebuffers();
  TRACE("Swapchain Framebuffers created");

  for (u32 frame_i = 0; frame_i < MAX_FRAMES_IN_FLIGHT; frame_i++) {
    context.main_cmd_bufs[frame_i] = gpu_cmd_encoder_create();
  }
  TRACE("main Command Buffer created");

  TRACE("Graphics pipeline created");
  return pipeline;
}

void gpu_pipeline_destroy(gpu_pipeline* pipeline) {
  vkDestroyPipeline(context.device->logical_device, pipeline->handle, context.allocator);
  vkDestroyPipelineLayout(context.device->logical_device, pipeline->layout_handle,
                          context.allocator);
}

gpu_cmd_encoder* gpu_get_default_cmd_encoder() {
  return &context.main_cmd_bufs[context.current_frame];
}

gpu_renderpass* gpu_renderpass_create(const gpu_renderpass_desc* description) {
  // TEMP: allocate with malloc. in the future we will have a pool allocator on the context
  gpu_renderpass* renderpass = malloc(sizeof(gpu_renderpass));

  // Colour attachment
  VkAttachmentDescription color_attachment;
  color_attachment.format = context.swapchain->image_format.format;
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
  // renderpass dependencies
  VkSubpassDependency dependency;
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = 0;

  // Finally, create the RenderPass
  VkRenderPassCreateInfo render_pass_create_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  render_pass_create_info.attachmentCount = 1;
  render_pass_create_info.pAttachments = &color_attachment;
  render_pass_create_info.subpassCount = 1;
  render_pass_create_info.pSubpasses = &subpass;
  render_pass_create_info.dependencyCount = 1;
  render_pass_create_info.pDependencies = &dependency;
  render_pass_create_info.flags = 0;
  render_pass_create_info.pNext = 0;

  VK_CHECK(vkCreateRenderPass(context.device->logical_device, &render_pass_create_info,
                              context.allocator, &renderpass->handle));

  // HACK
  context.main_renderpass = renderpass->handle;

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

gpu_cmd_encoder gpu_cmd_encoder_create() {
  // gpu_cmd_encoder* encoder = malloc(sizeof(gpu_cmd_encoder)); // TODO: fix leaking mem
  gpu_cmd_encoder encoder = { 0 };

  VkCommandBufferAllocateInfo allocate_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocate_info.commandPool = context.device->pool;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount = 1;
  allocate_info.pNext = NULL;

  VK_CHECK(vkAllocateCommandBuffers(context.device->logical_device, &allocate_info,
                                    &encoder.cmd_buffer););

  VkDescriptorPoolSize pool_sizes[2];
  // Uniforms pool
  pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_sizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT * MAX_DESCRIPTOR_SETS;
    // Samplers pool
  pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_sizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT * MAX_DESCRIPTOR_SETS;

  VkDescriptorPoolCreateInfo pool_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
  pool_info.poolSizeCount = 2;
  pool_info.pPoolSizes = pool_sizes;
  pool_info.maxSets = 100;

  VK_CHECK(vkCreateDescriptorPool(context.device->logical_device, &pool_info, context.allocator,
                                  &encoder.descriptor_pool));

  return encoder;
}
void gpu_cmd_encoder_destroy(gpu_cmd_encoder* encoder) {
  vkFreeCommandBuffers(context.device->logical_device, context.device->pool, 1,
                       &encoder->cmd_buffer);
}

void gpu_cmd_encoder_begin(gpu_cmd_encoder encoder) {
  VK_CHECK(vkResetDescriptorPool(context.device->logical_device, encoder.descriptor_pool, 0));

  VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  VK_CHECK(vkBeginCommandBuffer(encoder.cmd_buffer, &begin_info));
}

void gpu_cmd_encoder_begin_render(gpu_cmd_encoder* encoder, gpu_renderpass* renderpass) {
  VkRenderPassBeginInfo begin_info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  begin_info.renderPass = renderpass->handle;
  /* printf("Current img: %d Current frame %d\n", context.current_img_index, context.current_frame);
   */
  begin_info.framebuffer = context.swapchain_framebuffers[context.current_img_index];
  begin_info.renderArea.offset = (VkOffset2D){ 0, 0 };
  begin_info.renderArea.extent = context.swapchain->extent;

  // VkClearValue clear_values[2];
  VkClearValue clear_color = { { { 0.02f, 0.02f, 0.02f, 1.0f } } };
  // clear_values[1].depthStencil.depth = renderpass->depth;
  // clear_values[1].depthStencil.stencil = renderpass->stencil;

  begin_info.clearValueCount = 1;
  begin_info.pClearValues = &clear_color;

  vkCmdBeginRenderPass(encoder->cmd_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
  // command_buffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void gpu_cmd_encoder_end_render(gpu_cmd_encoder* encoder) {
  vkCmdEndRenderPass(encoder->cmd_buffer);
}

gpu_cmd_buffer gpu_cmd_encoder_finish(gpu_cmd_encoder* encoder) {
  vkEndCommandBuffer(encoder->cmd_buffer);

  // TEMP: submit
  return (gpu_cmd_buffer){ .cmd_buffer = encoder->cmd_buffer };
}

// --- Binding
void encode_bind_pipeline(gpu_cmd_encoder* encoder, pipeline_kind kind, gpu_pipeline* pipeline) {
  vkCmdBindPipeline(encoder->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
  encoder->pipeline = pipeline;
}

void encode_bind_shader_data(gpu_cmd_encoder* encoder, u32 group, shader_data* data) {
  arena tmp = arena_create(malloc(1024), 1024);

  assert(data->data != NULL);

  // Update the local buffer
  desc_set_uniform_buffer ubo = encoder->pipeline->uniform_pointers[group];
  memcpy(ubo.uniform_buf_mem_mappings[context.current_frame], data->data, ubo.size);

  VkDescriptorSetAllocateInfo alloc_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  alloc_info.descriptorPool = encoder->descriptor_pool;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts = &encoder->pipeline->desc_set_layouts[group];

  shader_data_layout sdl = data->shader_data_get_layout(data->data);
  size_t binding_count = sdl.bindings_count;
  assert(binding_count == 2);

  VkDescriptorSet sets[0];
  VK_CHECK(vkAllocateDescriptorSets(context.device->logical_device, &alloc_info, sets));
  // FIXME: hardcoded
  VkDescriptorSet_darray_push(context.free_set_queue, sets[0]);
  /* VkDescriptorSet_darray_push(context.free_set_queue, sets[1]); */

  VkWriteDescriptorSet write_sets[binding_count];
  memset(&write_sets, 0, binding_count * sizeof(VkWriteDescriptorSet));

  for (u32 i = 0; i < sdl.bindings_count; i++) {
    shader_binding binding = sdl.bindings[i];

    if (binding.type == SHADER_BINDING_BUFFER || binding.type == SHADER_BINDING_BYTES) {
      VkDescriptorBufferInfo* buffer_info = arena_alloc(&tmp, sizeof(VkDescriptorBufferInfo));
      buffer_info->buffer = ubo.buffers[context.current_frame];
      buffer_info->offset = 0;
      buffer_info->range = binding.data.bytes.size;

      write_sets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write_sets[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write_sets[i].descriptorCount = 1;
      write_sets[i].dstSet = sets[0];
      write_sets[i].dstBinding = i;
      write_sets[i].dstArrayElement = 0;
      write_sets[i].pBufferInfo = buffer_info;
    } else if (binding.type == SHADER_BINDING_TEXTURE) {
      gpu_texture* texture = TEXTURE_GET(binding.data.texture.handle);
      VkDescriptorImageInfo* image_info = arena_alloc(&tmp, sizeof(VkDescriptorImageInfo));
      image_info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      image_info->imageView = texture->view;
      image_info->sampler = texture->sampler;

      write_sets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write_sets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      write_sets[i].descriptorCount = 1;
      write_sets[i].dstSet = sets[0];
      write_sets[i].dstBinding = i;
      write_sets[i].dstArrayElement = 0;
      write_sets[i].pImageInfo = image_info;
    }else {
      WARN("Unknown binding");
    }
  }

  // Update
  vkUpdateDescriptorSets(context.device->logical_device, binding_count, write_sets, 0, NULL);

  // Bind
  vkCmdBindDescriptorSets(encoder->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          encoder->pipeline->layout_handle, 0, 1, sets, 0, NULL);

  arena_free_storage(&tmp);
}

void encode_set_vertex_buffer(gpu_cmd_encoder* encoder, buffer_handle buf) {
  gpu_buffer* buffer = BUFFER_GET(buf);  // context.buffers[buf.raw];
  VkBuffer vbs[] = { buffer->handle };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(encoder->cmd_buffer, 0, 1, vbs, offsets);
}

void encode_set_index_buffer(gpu_cmd_encoder* encoder, buffer_handle buf) {
  gpu_buffer* buffer = BUFFER_GET(buf);  // context.buffers[buf.raw];
  vkCmdBindIndexBuffer(encoder->cmd_buffer, buffer->handle, 0, VK_INDEX_TYPE_UINT32);
}

// TEMP
void encode_set_default_settings(gpu_cmd_encoder* encoder) {
  VkViewport viewport = { 0 };
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = context.swapchain->extent.width;
  viewport.height = context.swapchain->extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(encoder->cmd_buffer, 0, 1, &viewport);

  VkRect2D scissor = { 0 };
  scissor.offset = (VkOffset2D){ 0, 0 };
  scissor.extent = context.swapchain->extent;
  vkCmdSetScissor(encoder->cmd_buffer, 0, 1, &scissor);
}

// --- Drawing

bool gpu_backend_begin_frame() {
  u32 current_frame = context.current_frame;
  vkWaitForFences(context.device->logical_device, 1, &context.in_flight_fences[current_frame],
                  VK_TRUE, UINT64_MAX);

  u32 image_index;
  VkResult result = vkAcquireNextImageKHR(
      context.device->logical_device, context.swapchain->handle, UINT64_MAX,
      context.image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || context.is_resizing) {
    ERROR("Acquire next image failure. recreate swapchain");
    context.is_resizing = false;
    recreate_swapchain(context.swapchain);
    return false;
  } else if (result != VK_SUCCESS) {
    ERROR_EXIT("failed to acquire swapchain image");
  }

  vkResetFences(context.device->logical_device, 1, &context.in_flight_fences[current_frame]);

  context.current_img_index = image_index;
  VK_CHECK(vkResetCommandBuffer(context.main_cmd_bufs[current_frame].cmd_buffer, 0));
  return true;
}

void gpu_temp_draw(size_t n_indices) {
  gpu_cmd_encoder* encoder = gpu_get_default_cmd_encoder();  // &context.main_cmd_buf;
  /* vkCmdDraw(encoder->cmd_buffer, n_verts, 1, 0, 0); */
  vkCmdDrawIndexed(encoder->cmd_buffer, n_indices, 1, 0, 0, 0);
}

void gpu_backend_end_frame() {
  VkPresentInfoKHR present_info = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &context.render_finished_semaphores[context.current_frame];

  VkSwapchainKHR swapchains[] = { context.swapchain->handle };
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swapchains;
  present_info.pImageIndices = &context.current_img_index;

  VkResult result = vkQueuePresentKHR(context.device->present_queue, &present_info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    ERROR("Queue present error. recreate swapchain");
    recreate_swapchain(context.swapchain);
    return;
  } else if (result != VK_SUCCESS) {
    ERROR_EXIT("failed to present swapchain image");
  }
  context.current_frame = (context.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

  /* vkDeviceWaitIdle(context.device->logical_device); */
}

// TODO: Move into better order in file
void gpu_queue_submit(gpu_cmd_buffer* buffer) {
  VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

  // Specify semaphore to wait on
  VkSemaphore wait_semaphores[] = { context.image_available_semaphores[context.current_frame] };
  VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;

  // Specify semaphore to signal when finished executing buffer
  VkSemaphore signal_semaphores[] = { context.render_finished_semaphores[context.current_frame] };
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &buffer->cmd_buffer;

  VK_CHECK(vkQueueSubmit(context.device->graphics_queue, 1, &submit_info,
                         context.in_flight_fences[context.current_frame]));
}

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

  vkGetPhysicalDeviceProperties(out_device->physical_device, &out_device->properties);
  vkGetPhysicalDeviceFeatures(out_device->physical_device, &out_device->features);
  vkGetPhysicalDeviceMemoryProperties(out_device->physical_device, &out_device->memory);

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

  vulkan_device_query_swapchain_support(device, context.surface, &context.swapchain_support);

  return indices.has_graphics && indices.has_present && context.swapchain_support.mode_count > 0 &&
         context.swapchain_support.format_count > 0;
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
    if (present_support && !indices.has_present) {
      indices.present_family_index = q_fam_i;
      indices.has_present = true;
    }
  }

  return indices;
}

bool create_logical_device(gpu_device* out_device) {
  queue_family_indices indices = find_queue_families(out_device->physical_device);
  INFO(" %s | %s | %s | %s | %s", bool_str(indices.has_graphics), bool_str(indices.has_present),
       bool_str(indices.has_compute), bool_str(indices.has_transfer),
       out_device->properties.deviceName);
  TRACE("Graphics Family queue index: %d", indices.graphics_family_index);
  TRACE("Present Family queue index: %d", indices.present_family_index);
  TRACE("Compute Family queue index: %d", indices.compute_family_index);
  TRACE("Transfer Family queue index: %d", indices.transfer_family_index);

  // Queues
  f32 prio_one = 1.0;
  VkDeviceQueueCreateInfo queue_create_infos[1] = { 0 };
  queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_infos[0].queueFamilyIndex = indices.graphics_family_index;
  queue_create_infos[0].queueCount = 1;
  queue_create_infos[0].pQueuePriorities = &prio_one;
  queue_create_infos[0].flags = 0;
  queue_create_infos[0].pNext = 0;

  // queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  // queue_create_infos[1].queueFamilyIndex = indices.present_family_index;
  // queue_create_infos[1].queueCount = 1;
  // queue_create_infos[1].pQueuePriorities = &prio_one;
  // queue_create_infos[1].flags = 0;
  // queue_create_infos[1].pNext = 0;

  // Features
  VkPhysicalDeviceFeatures device_features = { 0 };
  device_features.samplerAnisotropy = VK_TRUE;  // request anistrophy

  // Device itself
  VkDeviceCreateInfo device_create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
  device_create_info.queueCreateInfoCount = 1;
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

VkShaderModule create_shader_module(str8 spirv) {
  VkShaderModuleCreateInfo create_info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  create_info.codeSize = spirv.len;
  create_info.pCode = (uint32_t*)spirv.buf;

  VkShaderModule shader_module;
  VK_CHECK(vkCreateShaderModule(context.device->logical_device, &create_info, context.allocator,
                                &shader_module));

  return shader_module;
}

void create_descriptor_pools() {}

void create_swapchain_framebuffers() {
  WARN("Recreating framebuffers...");
  u32 image_count = context.swapchain->image_count;
  context.swapchain_framebuffers =
      arena_alloc(&context.swapchain->swapchain_arena, image_count * sizeof(VkFramebuffer));
  for (u32 i = 0; i < image_count; i++) {
    VkImageView attachments[1] = { context.swapchain->image_views[i] };

    VkFramebufferCreateInfo framebuffer_create_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    framebuffer_create_info.attachmentCount = 1;
    framebuffer_create_info.pAttachments = attachments;

    framebuffer_create_info.renderPass =
        context.main_renderpass;  // TODO:  description.renderpass->handle;
    framebuffer_create_info.width = context.swapchain->extent.width;
    framebuffer_create_info.height = context.swapchain->extent.height;
    framebuffer_create_info.layers = 1;

    VK_CHECK(vkCreateFramebuffer(context.device->logical_device, &framebuffer_create_info,
                                 context.allocator, &context.swapchain_framebuffers[i]));
  }
}

void create_sync_objects() {
  VkSemaphoreCreateInfo semaphore_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
  VkFenceCreateInfo fence_info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VK_CHECK(vkCreateSemaphore(context.device->logical_device, &semaphore_info, context.allocator,
                               &context.image_available_semaphores[i]););
    VK_CHECK(vkCreateSemaphore(context.device->logical_device, &semaphore_info, context.allocator,
                               &context.render_finished_semaphores[i]););

    VK_CHECK(vkCreateFence(context.device->logical_device, &fence_info, context.allocator,
                           &context.in_flight_fences[i]));
  }
}

static i32 find_memory_index(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(context.device->physical_device, &memory_properties);

  for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
    // Check each memory type to see if its bit is set to 1.
    if (type_filter & (1 << i) &&
        (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
      return i;
    }
  }

  WARN("Unable to find suitable memory type!");
  return -1;
}

buffer_handle gpu_buffer_create(u64 size, gpu_buffer_type buf_type, gpu_buffer_flags flags,
                                const void* data) {
  VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  buffer_info.size = size;
  buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  switch (buf_type) {
    case CEL_BUFFER_DEFAULT:
      buffer_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
      break;
    case CEL_BUFFER_VERTEX:
      buffer_info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
      break;
    case CEL_BUFFER_INDEX:
      buffer_info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
      break;
    case CEL_BUFFER_UNIFORM:
      buffer_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      break;
    case CEL_BUFFER_COUNT:
      WARN("Incorrect gpu_buffer_type provided. using default");
      break;
  }

  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  // "allocating" the cpu-side buffer struct
  /* gpu_buffer buffer; */
  /* buffer.size = size; */
  buffer_handle handle;
  gpu_buffer* buffer = buffer_pool_alloc(&context.resource_pools->buffers, &handle);
  buffer->size = size;

  VK_CHECK(vkCreateBuffer(context.device->logical_device, &buffer_info, context.allocator,
                          &buffer->handle));

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(context.device->logical_device, buffer->handle, &requirements);

  // Just make them always need all of them for now
  i32 memory_index =
      find_memory_index(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  // Allocate the actual VRAM
  VkMemoryAllocateInfo allocate_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  allocate_info.allocationSize = requirements.size;
  allocate_info.memoryTypeIndex = (u32)memory_index;

  vkAllocateMemory(context.device->logical_device, &allocate_info, context.allocator,
                   &buffer->memory);
  vkBindBufferMemory(context.device->logical_device, buffer->handle, buffer->memory, 0);

  /* Now there are two options:
   *   1. create CPU-accessible memory -> map memory -> memcpy -> unmap
   *   2. use a staging buffer thats CPU-accessible and copy its contents to a
   *      GPU-only buffer
   */

  /* context.buffers[context.buffer_count] = buffer; */
  /* context.buffer_count++; */

  if (data) {
    TRACE("Upload data as part of buffer creation");
    if (flags & CEL_BUFFER_FLAG_CPU) {
      // map memory -> copy data in -> unmap memory
      buffer_upload_bytes(handle, (bytebuffer){ .buf = (u8*)data, .size = size }, 0, size);
    } else if (flags & CEL_BUFFER_FLAG_GPU) {
      TRACE("Uploading data to buffer using staging buffer");
      // Create a staging buffer
      buffer_handle staging = gpu_buffer_create(size, buf_type, CEL_BUFFER_FLAG_CPU, NULL);

      // Copy data into it
      buffer_upload_bytes(staging, (bytebuffer){ .buf = (u8*)data, .size = size }, 0, size);

      // Enqueue a copy from the staging buffer into the DEVICE_LOCAL buffer
      gpu_cmd_encoder temp_encoder = gpu_cmd_encoder_create();
      gpu_cmd_encoder_begin(temp_encoder);
      encode_buffer_copy(&temp_encoder, staging, 0, handle, 0, size);
      gpu_cmd_buffer copy_cmd_buffer = gpu_cmd_encoder_finish(&temp_encoder);

      VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
      submit_info.commandBufferCount = 1;
      submit_info.pCommandBuffers = &temp_encoder.cmd_buffer;
      vkQueueSubmit(context.device->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);

      // Cleanup
      vkQueueWaitIdle(context.device->graphics_queue);
      gpu_cmd_encoder_destroy(&temp_encoder);
      gpu_buffer_destroy(staging);
    }
  }

  return handle;
}

void gpu_buffer_destroy(buffer_handle buffer) {
  gpu_buffer* b = buffer_pool_get(&context.resource_pools->buffers, buffer);
  vkDestroyBuffer(context.device->logical_device, b->handle, context.allocator);
  vkFreeMemory(context.device->logical_device, b->memory, context.allocator);
  buffer_pool_dealloc(&context.resource_pools->buffers, buffer);
}

// Upload data to a
void buffer_upload_bytes(buffer_handle gpu_buf, bytebuffer cpu_buf, u64 offset, u64 size) {
  gpu_buffer* buffer = buffer_pool_get(&context.resource_pools->buffers, gpu_buf);
  void* data_ptr;
  vkMapMemory(context.device->logical_device, buffer->memory, 0, size, 0, &data_ptr);
  DEBUG("Uploading %d bytes to buffer", size);
  memcpy(data_ptr, cpu_buf.buf, size);
  vkUnmapMemory(context.device->logical_device, buffer->memory);
}

void encode_buffer_copy(gpu_cmd_encoder* encoder, buffer_handle src, u64 src_offset,
                        buffer_handle dst, u64 dst_offset, u64 copy_size) {
  VkBufferCopy copy_region;
  copy_region.srcOffset = src_offset;
  copy_region.dstOffset = dst_offset;
  copy_region.size = copy_size;

  gpu_buffer* src_buf = buffer_pool_get(&context.resource_pools->buffers, src);
  gpu_buffer* dst_buf = buffer_pool_get(&context.resource_pools->buffers, dst);
  vkCmdCopyBuffer(encoder->cmd_buffer, src_buf->handle, dst_buf->handle, 1, &copy_region);
}

// one-shot command buffers
VkCommandBuffer vulkan_command_buffer_create_oneshot() {
  VkCommandBufferAllocateInfo alloc_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  alloc_info.commandPool = context.device->pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;
  alloc_info.pNext = 0;

  VkCommandBuffer cmd_buffer;
  vkAllocateCommandBuffers(context.device->logical_device, &alloc_info, &cmd_buffer);

  VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmd_buffer, &begin_info);

  return cmd_buffer;
}

void vulkan_command_buffer_finish_oneshot(VkCommandBuffer cmd_buffer) {
  VK_CHECK(vkEndCommandBuffer(cmd_buffer));

  // submit to queue
  VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd_buffer;
  VK_CHECK(vkQueueSubmit(context.device->graphics_queue, 1, &submit_info, 0));
  VK_CHECK(vkQueueWaitIdle(context.device->graphics_queue));

  vkFreeCommandBuffers(context.device->logical_device, context.device->pool, 1, &cmd_buffer);
}

void copy_buffer_to_buffer_oneshot(buffer_handle src, u64 src_offset, buffer_handle dst,
                                   u64 dst_offset, u64 copy_size) {
  VkBufferCopy copy_region;
  copy_region.srcOffset = src_offset;
  copy_region.dstOffset = dst_offset;
  copy_region.size = copy_size;

  gpu_buffer* src_buf = buffer_pool_get(&context.resource_pools->buffers, src);
  gpu_buffer* dst_buf = buffer_pool_get(&context.resource_pools->buffers, dst);
  VkCommandBuffer temp_cmd_buffer = vulkan_command_buffer_create_oneshot();
  vkCmdCopyBuffer(temp_cmd_buffer, src_buf->handle, dst_buf->handle, 1, &copy_region);
  vulkan_command_buffer_finish_oneshot(temp_cmd_buffer);
}

void copy_buffer_to_image_oneshot(buffer_handle src, texture_handle dst) {
  gpu_buffer* src_buf = buffer_pool_get(&context.resource_pools->buffers, src);
  gpu_texture* dst_tex = texture_pool_get(&context.resource_pools->textures, dst);

  VkCommandBuffer temp_cmd_buffer = vulkan_command_buffer_create_oneshot();

  VkBufferImageCopy region;
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  printf("Image details width: %d height %d\n", dst_tex->desc.extents.x, dst_tex->desc.extents.y);
  region.imageOffset.x = 0;
  region.imageOffset.y = 0;
  region.imageOffset.z = 0;
  region.imageExtent.width = dst_tex->desc.extents.x;
  region.imageExtent.height = dst_tex->desc.extents.y;
  region.imageExtent.depth = 1;

  vkCmdCopyBufferToImage(temp_cmd_buffer, src_buf->handle, dst_tex->handle,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  vulkan_command_buffer_finish_oneshot(temp_cmd_buffer);
}

texture_handle gpu_texture_create(texture_desc desc, bool create_view, const void* data) {
  VkDeviceSize image_size = desc.extents.x * desc.extents.y * 4;

  VkImage image;
  VkDeviceMemory image_memory;

  // FIXME: get from desc
  VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

  VkImageCreateInfo image_create_info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
  image_create_info.imageType = VK_IMAGE_TYPE_2D;
  image_create_info.extent.width = desc.extents.x;
  image_create_info.extent.height = desc.extents.y;
  image_create_info.extent.depth = 1;
  image_create_info.mipLevels = 1;
  image_create_info.arrayLayers = 1;
  image_create_info.format = format;
  image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;

  VK_CHECK(
      vkCreateImage(context.device->logical_device, &image_create_info, context.allocator, &image));

  VkMemoryRequirements memory_reqs;
  vkGetImageMemoryRequirements(context.device->logical_device, image, &memory_reqs);

  VkMemoryAllocateInfo alloc_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  alloc_info.allocationSize = memory_reqs.size;
  alloc_info.memoryTypeIndex =
      find_memory_index(memory_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  vkAllocateMemory(context.device->logical_device, &alloc_info, context.allocator, &image_memory);

  vkBindImageMemory(context.device->logical_device, image, image_memory, 0);

  texture_handle handle;
  gpu_texture* texture = texture_pool_alloc(&context.resource_pools->textures, &handle);
  DEBUG("Allocated texture with handle %d", handle.raw);
  texture->handle = image;
  texture->debug_label = "Test Texture";
  texture->desc = desc;
  texture->memory = image_memory;
  texture->size = image_size;

  if (data) {
    TRACE("Uploading pixel data to texture using staging buffer");
    // Create a staging buffer
    buffer_handle staging =
        gpu_buffer_create(image_size, CEL_BUFFER_DEFAULT, CEL_BUFFER_FLAG_CPU, NULL);
    // Copy data into it
    vulkan_transition_image_layout(texture, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    buffer_upload_bytes(staging, (bytebuffer){ .buf = (u8*)data, .size = image_size }, 0, image_size);
    copy_buffer_to_image_oneshot(staging, handle);
    vulkan_transition_image_layout(texture, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    gpu_buffer_destroy(staging);
  }

  // Texture View
  if (create_view) {
    VkImageViewCreateInfo view_create_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    view_create_info.image = image;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = format;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context.device->logical_device, &view_create_info, context.allocator,
                               &texture->view));
  }

  // Sampler
    VkSamplerCreateInfo sampler_info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = 16;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0;
    sampler_info.minLod = 0.0;
    sampler_info.maxLod = 0.0;

    VkResult res = vkCreateSampler(context.device->logical_device, &sampler_info, context.allocator,
                                   &texture->sampler);
    if (res != VK_SUCCESS) {
      ERROR("Error creating texture sampler for image %s", texture->debug_label);
      return;
    }

  return handle;
}

void vulkan_transition_image_layout(gpu_texture* texture, VkFormat format, VkImageLayout old_layout,
                                    VkImageLayout new_layout) {
  VkCommandBuffer temp_cmd_buffer = vulkan_command_buffer_create_oneshot();

  VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = context.device->queue_family_indicies.graphics_family_index;
  barrier.dstQueueFamilyIndex = context.device->queue_family_indicies.graphics_family_index;
  barrier.image = texture->handle;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = 0;  // TODO
  barrier.dstAccessMask = 0;  // TODO

  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags dest_stage;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    FATAL("Unsupported image layout transition");
    return;
  }

  vkCmdPipelineBarrier(temp_cmd_buffer, source_stage, dest_stage, 0, 0, 0, 0, 0, 1, &barrier);

  vulkan_command_buffer_finish_oneshot(temp_cmd_buffer);
}

size_t vertex_attrib_size(vertex_attrib_type attr) {
  switch (attr) {
    case ATTR_F32:
    case ATTR_U32:
    case ATTR_I32:
      return 4;
    case ATTR_F32x2:
    case ATTR_U32x2:
    case ATTR_I32x2:
      return 8;
    case ATTR_F32x3:
    case ATTR_U32x3:
    case ATTR_I32x3:
      return 12;
    case ATTR_F32x4:
    case ATTR_U32x4:
    case ATTR_I32x4:
      return 16;
      break;
  }
}

void vertex_desc_add(vertex_description* builder, const char* name, vertex_attrib_type type) {
  u32 i = builder->attributes_count;

  size_t size = vertex_attrib_size(type);
  builder->attributes[i] = type;
  builder->stride += size;
  builder->attr_names[i] = name;

  builder->attributes_count++;
}

/* TYPED_POOL(gpu_buffer, buffer); */
/* TYPED_POOL(gpu_texture, texture); */

void resource_pools_init(arena* a, struct resource_pools* res_pools) {
  buffer_pool buf_pool = buffer_pool_create(a, MAX_BUFFERS, sizeof(gpu_buffer));
  res_pools->buffers = buf_pool;
  texture_pool tex_pool = texture_pool_create(a, MAX_TEXTURES, sizeof(gpu_texture));
  res_pools->textures = tex_pool;

  context.resource_pools = res_pools;
}
