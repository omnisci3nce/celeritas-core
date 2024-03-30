#define CDEBUG
#define CEL_PLATFORM_LINUX
// ^ Temporary

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "colours.h"
#include "str.h"

#include "darray.h"
#include "defines.h"
#include "file.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "render_backend.h"
#include "render_types.h"
#include "vulkan_helpers.h"

#include <stdlib.h>

#define SCR_WIDTH 1000
#define SCR_HEIGHT 1000

#if CEL_REND_BACKEND_VULKAN

#include <glad/glad.h>

#include <glfw3.h>

KITC_DECL_TYPED_ARRAY(VkLayerProperties)

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
  VkCommandPool gfx_command_pool;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
  VkFormat depth_format;
} vulkan_device;

typedef struct vulkan_image {
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  u32 width;
  u32 height;
} vulkan_image;

typedef enum vulkan_renderpass_state {
  READY,
  RECORDING,
  IN_RENDER_PASS,
  RECORDING_ENDING,
  SUBMITTED,
  NOT_ALLOCATED
} vulkan_renderpass_state;

typedef struct vulkan_renderpass {
  VkRenderPass handle;
  vec4 render_area;
  vec4 clear_colour;
  f32 depth;
  u32 stencil;
  vulkan_renderpass_state state;
} vulkan_renderpass;

typedef struct vulkan_framebuffer {
  VkFramebuffer handle;
  u32 attachment_count;
  VkImageView* attachments;
  vulkan_renderpass* renderpass;
} vulkan_framebuffer;

KITC_DECL_TYPED_ARRAY(vulkan_framebuffer)

typedef struct vulkan_swapchain {
  VkSurfaceFormatKHR image_format;
  u8 max_frames_in_flight;
  VkSwapchainKHR handle;
  u32 image_count;
  VkImage* images;
  VkImageView* views;
  vulkan_image depth_attachment;
  vulkan_framebuffer_darray* framebuffers;
} vulkan_swapchain;

typedef enum vulkan_command_buffer_state {
  COMMAND_BUFFER_STATE_READY,
  COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  COMMAND_BUFFER_STATE_RECORDING,
  COMMAND_BUFFER_STATE_RECORDING_ENDED,
  COMMAND_BUFFER_STATE_SUBMITTED,
  COMMAND_BUFFER_STATE_NOT_ALLOCATED,
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer {
  VkCommandBuffer handle;
  vulkan_command_buffer_state state;
} vulkan_command_buffer;

KITC_DECL_TYPED_ARRAY(vulkan_command_buffer)

typedef struct vulkan_fence {
  VkFence handle;
  bool is_signaled;
} vulkan_fence;

typedef struct vulkan_shader_stage {
  VkShaderModuleCreateInfo create_info;
  VkShaderModule handle;
  VkPipelineShaderStageCreateInfo stage_create_info;
} vulkan_shader_stage;

typedef struct vulkan_pipeline {
  VkPipeline handle;
  VkPipelineLayout layout;
} vulkan_pipeline;

typedef struct global_object_uniform {
  mat4 projection;  // 64 bytes
  mat4 view;        // 64 bytes
  f32 padding[32];
} global_object_uniform;

typedef struct vulkan_buffer {
  u64 total_size;
  VkBuffer handle;
  VkBufferUsageFlagBits usage;
  bool is_locked;
  VkDeviceMemory memory;
  i32 memory_index;
  u32 memory_property_flags;
} vulkan_buffer;

#define SHADER_STAGE_COUNT 2

typedef struct vulkan_shader {
  // vertex, fragment
  vulkan_shader_stage stages[SHADER_STAGE_COUNT];
  vulkan_pipeline pipeline;

  // descriptors
  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSet descriptor_sets[3];  // one for each in-flight frame

  vulkan_buffer global_uniforms_buffer;

  // Data that's global for all objects drawn
  global_object_uniform global_ubo;
} vulkan_shader;

typedef struct vulkan_context {
  VkInstance instance;
  VkAllocationCallbacks* allocator;
  VkSurfaceKHR surface;
  vulkan_device device;
  u32 framebuffer_width;
  u32 framebuffer_height;
  vulkan_swapchain swapchain;
  vulkan_renderpass main_renderpass;
  vulkan_buffer object_vertex_buffer;
  vulkan_buffer object_index_buffer;
  u64 geometry_vertex_offset;
  u64 geometry_index_offset;

  vulkan_command_buffer_darray* gfx_command_buffers;

  VkSemaphore* image_available_semaphores;
  VkSemaphore* queue_complete_semaphores;
  u32 in_flight_fence_count;
  vulkan_fence* in_flight_fences;
  vulkan_fence** images_in_flight;

  u32 image_index;
  u32 current_frame;

  vulkan_shader object_shader;

  // TODO: swapchain recreation

#if defined(DEBUG)
  VkDebugUtilsMessengerEXT vk_debugger;
#endif
} vulkan_context;

static vulkan_context context;

static i32 find_memory_index(vulkan_context* context, u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(context->device.physical_device, &memory_properties);

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

/** @brief Internal backend state */
typedef struct vulkan_state {
} vulkan_state;

typedef struct vertex_pos {
  vec3 pos;
} vertex_pos;

// pipeline stuff
bool vulkan_graphics_pipeline_create(vulkan_context* context, vulkan_renderpass* renderpass,
                                     u32 attribute_count,
                                     VkVertexInputAttributeDescription* attributes,
                                     u32 descriptor_set_layout_count,
                                     VkDescriptorSetLayout* descriptor_set_layouts, u32 stage_count,
                                     VkPipelineShaderStageCreateInfo* stages, VkViewport viewport,
                                     VkRect2D scissor, bool is_wireframe,
                                     vulkan_pipeline* out_pipeline) {
  VkPipelineViewportStateCreateInfo viewport_state = {
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
  };
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
  };
  rasterizer_create_info.depthClampEnable = VK_FALSE;
  rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
  rasterizer_create_info.polygonMode = is_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
  rasterizer_create_info.lineWidth = 1.0f;
  rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

  // Depth and stencil testing
  VkPipelineDepthStencilStateCreateInfo depth_stencil = {
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
  };
  depth_stencil.depthTestEnable = VK_TRUE;
  depth_stencil.depthWriteEnable = VK_TRUE;
  depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depth_stencil.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.stencilTestEnable = VK_FALSE;
  depth_stencil.pNext = 0;

  VkPipelineColorBlendAttachmentState color_blend_attachment_state;
  color_blend_attachment_state.blendEnable = VK_TRUE;
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

  const u32 dynamic_state_count = 3;
  VkDynamicState dynamic_states[3] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    VK_DYNAMIC_STATE_LINE_WIDTH,
  };

  VkPipelineDynamicStateCreateInfo dynamic_state = {
    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
  };
  dynamic_state.dynamicStateCount = dynamic_state_count;
  dynamic_state.pDynamicStates = dynamic_states;

  // Vertex input
  VkVertexInputBindingDescription binding_desc;
  binding_desc.binding = 0;
  binding_desc.stride = sizeof(vertex_pos);
  binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkPipelineVertexInputStateCreateInfo vertex_input_info = {
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
  };
  vertex_input_info.vertexBindingDescriptionCount = 1;
  vertex_input_info.pVertexBindingDescriptions = &binding_desc;
  vertex_input_info.vertexAttributeDescriptionCount = attribute_count;
  vertex_input_info.pVertexAttributeDescriptions = attributes;

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
  };
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
  };
  pipeline_layout_create_info.setLayoutCount = descriptor_set_layout_count;
  pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts;

  vkCreatePipelineLayout(context->device.logical_device, &pipeline_layout_create_info,
                         context->allocator, &out_pipeline->layout);

  VkGraphicsPipelineCreateInfo pipeline_create_info = {
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
  };
  pipeline_create_info.stageCount = stage_count;
  pipeline_create_info.pStages = stages;
  pipeline_create_info.pVertexInputState = &vertex_input_info;
  pipeline_create_info.pInputAssemblyState = &input_assembly;

  pipeline_create_info.pViewportState = &viewport_state;
  pipeline_create_info.pRasterizationState = &rasterizer_create_info;
  pipeline_create_info.pMultisampleState = &ms_create_info;
  pipeline_create_info.pDepthStencilState = &depth_stencil;
  pipeline_create_info.pColorBlendState = &color_blend;
  pipeline_create_info.pDynamicState = &dynamic_state;
  pipeline_create_info.pTessellationState = 0;

  pipeline_create_info.layout = out_pipeline->layout;

  pipeline_create_info.renderPass = renderpass->handle;
  pipeline_create_info.subpass = 0;
  pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_create_info.basePipelineIndex = -1;

  VkResult result =
      vkCreateGraphicsPipelines(context->device.logical_device, VK_NULL_HANDLE, 1,
                                &pipeline_create_info, context->allocator, &out_pipeline->handle);
  if (result != VK_SUCCESS) {
    FATAL("graphics pipeline creation failed. its fked mate");
    ERROR_EXIT("Doomed");
  }

  return true;
}

void vulkan_pipeline_bind(vulkan_command_buffer* command_buffer, VkPipelineBindPoint bind_point,
                          vulkan_pipeline* pipeline) {
  vkCmdBindPipeline(command_buffer->handle, bind_point, pipeline->handle);
}

void vulkan_buffer_bind(vulkan_context* context, vulkan_buffer* buffer, u64 offset) {
  vkBindBufferMemory(context->device.logical_device, buffer->handle, buffer->memory, offset);
}

bool vulkan_buffer_create(vulkan_context* context, u64 size, VkBufferUsageFlagBits usage,
                          u32 memory_property_flags, bool bind_on_create,
                          vulkan_buffer* out_buffer) {
  memset(out_buffer, 0, sizeof(vulkan_buffer));
  out_buffer->total_size = size;
  out_buffer->usage = usage;
  out_buffer->memory_property_flags = memory_property_flags;

  VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  vkCreateBuffer(context->device.logical_device, &buffer_info, context->allocator,
                 &out_buffer->handle);

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(context->device.logical_device, out_buffer->handle, &requirements);
  out_buffer->memory_index =
      find_memory_index(context, requirements.memoryTypeBits, out_buffer->memory_property_flags);

  // Allocate
  VkMemoryAllocateInfo allocate_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  allocate_info.allocationSize = requirements.size;
  allocate_info.memoryTypeIndex = (u32)out_buffer->memory_index;

  vkAllocateMemory(context->device.logical_device, &allocate_info, context->allocator,
                   &out_buffer->memory);

  if (bind_on_create) {
    vulkan_buffer_bind(context, out_buffer, 0);
  }

  return true;
}

// lock and unlock?

void* vulkan_buffer_lock_memory(vulkan_context* context, vulkan_buffer* buffer, u64 offset,
                                u64 size, u32 flags) {
  void* data;
  vkMapMemory(context->device.logical_device, buffer->memory, offset, size, flags, &data);
  return data;
}
void* vulkan_buffer_unlock_memory(vulkan_context* context, vulkan_buffer* buffer) {
  vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void vulkan_buffer_load_data(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size,
                             u32 flags, const void* data) {
  void* data_ptr = 0;
  VK_CHECK(
      vkMapMemory(context->device.logical_device, buffer->memory, offset, size, flags, &data_ptr));
  memcpy(data_ptr, data, size);
  vkUnmapMemory(context->device.logical_device, buffer->memory);
}

// TODO: destroy

bool create_shader_module(vulkan_context* context, const char* filename, const char* type_str,
                          VkShaderStageFlagBits flag, u32 stage_index,
                          vulkan_shader_stage* shader_stages) {
  memset(&shader_stages[stage_index].create_info, 0, sizeof(VkShaderModuleCreateInfo));
  memset(&shader_stages[stage_index].stage_create_info, 0, sizeof(VkPipelineShaderStageCreateInfo));

  shader_stages[stage_index].create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

  // todo: file input
  FileData file_contents = load_spv_file(filename);

  shader_stages[stage_index].create_info.codeSize = file_contents.size;
  shader_stages[stage_index].create_info.pCode = (u32*)file_contents.data;

  vkCreateShaderModule(context->device.logical_device, &shader_stages[stage_index].create_info,
                       context->allocator, &shader_stages[stage_index].handle);

  shader_stages[stage_index].stage_create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[stage_index].stage_create_info.stage = flag;
  shader_stages[stage_index].stage_create_info.module = shader_stages[stage_index].handle;
  shader_stages[stage_index].stage_create_info.pName = "main";

  free(file_contents.data);

  // TODO: Descriptors

  return true;
}

bool vulkan_object_shader_create(vulkan_context* context, vulkan_shader* out_shader) {
  char stage_type_strs[SHADER_STAGE_COUNT][5] = { "vert", "frag" };
  char stage_filenames[SHADER_STAGE_COUNT][256] = { "build/linux/x86_64/debug/object.vert.spv",
                                                    "build/linux/x86_64/debug/object.frag.spv" };
  VkShaderStageFlagBits stage_types[SHADER_STAGE_COUNT] = { VK_SHADER_STAGE_VERTEX_BIT,
                                                            VK_SHADER_STAGE_FRAGMENT_BIT };
  for (u8 i = 0; i < SHADER_STAGE_COUNT; i++) {
    DEBUG("Loading %s", stage_filenames[i]);
    create_shader_module(context, stage_filenames[i], stage_type_strs[i], stage_types[i], i,
                         out_shader->stages);
  }

  // descriptors
  VkDescriptorSetLayoutBinding global_ubo_layout_binding;
  global_ubo_layout_binding.binding = 0;
  global_ubo_layout_binding.descriptorCount = 1;
  global_ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  global_ubo_layout_binding.pImmutableSamplers = 0;
  global_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo global_layout_info = {
    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
  };
  global_layout_info.bindingCount = 1;
  global_layout_info.pBindings = &global_ubo_layout_binding;

  VK_CHECK(vkCreateDescriptorSetLayout(context->device.logical_device, &global_layout_info,
                                       context->allocator, &out_shader->descriptor_set_layout));

  VkDescriptorPoolSize global_pool_size;
  global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  global_pool_size.descriptorCount = 3;

  VkDescriptorPoolCreateInfo pool_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
  pool_info.poolSizeCount = 1;
  pool_info.pPoolSizes = &global_pool_size;
  pool_info.maxSets = 3;

  VK_CHECK(vkCreateDescriptorPool(context->device.logical_device, &pool_info, context->allocator,
                                  &out_shader->descriptor_pool));

  // Pipeline creation
  VkViewport viewport;
  viewport.x = 0;
  viewport.y = (f32)context->framebuffer_height;
  viewport.width = (f32)context->framebuffer_width;
  viewport.height = -(f32)context->framebuffer_height;
  viewport.minDepth = 0.0;
  viewport.maxDepth = 1.0;

  VkRect2D scissor;
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width = context->framebuffer_width;
  scissor.extent.height = context->framebuffer_height;

  // Attributes
  u32 offset = 0;
  const i32 attribute_count = 1;
  VkVertexInputAttributeDescription attribute_descs[1];
  // Position
  VkFormat formats[1] = { VK_FORMAT_R32G32B32_SFLOAT };

  u64 sizes[1] = { sizeof(vec3) };

  for (u32 i = 0; i < attribute_count; i++) {
    attribute_descs[i].binding = 0;
    attribute_descs[i].location = i;
    attribute_descs[i].format = formats[i];
    attribute_descs[i].offset = offset;
    offset += sizes[i];
  }

  // Descriptor set layouts
  VkDescriptorSetLayout layouts[1] = { out_shader->descriptor_set_layout };

  // Stages
  VkPipelineShaderStageCreateInfo stage_create_infos[SHADER_STAGE_COUNT];
  memset(stage_create_infos, 0, sizeof(stage_create_infos));
  for (u32 i = 0; i < SHADER_STAGE_COUNT; i++) {
    stage_create_infos[i].sType = out_shader->stages[i].stage_create_info.sType;
    stage_create_infos[i] = out_shader->stages[i].stage_create_info;
  }

  vulkan_graphics_pipeline_create(
      context, &context->main_renderpass, attribute_count, attribute_descs, 1, layouts,
      SHADER_STAGE_COUNT, stage_create_infos, viewport, scissor, false, &out_shader->pipeline);
  INFO("Graphics pipeline created!");

  // Uniform buffer
  if (!vulkan_buffer_create(context, sizeof(global_object_uniform),
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            true, &out_shader->global_uniforms_buffer)) {
    ERROR("Couldnt create uniforms buffer");
    return false;
  }

  VkDescriptorSetLayout global_layouts[3] = {
    out_shader->descriptor_set_layout,
    out_shader->descriptor_set_layout,
    out_shader->descriptor_set_layout,
  };

  VkDescriptorSetAllocateInfo alloc_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  alloc_info.descriptorPool = out_shader->descriptor_pool;
  alloc_info.descriptorSetCount = 3;
  alloc_info.pSetLayouts = global_layouts;
  VK_CHECK(vkAllocateDescriptorSets(context->device.logical_device, &alloc_info,
                                    out_shader->descriptor_sets));

  return true;
}
void vulkan_object_shader_destroy(vulkan_context* context, vulkan_shader* shader) {}
void vulkan_object_shader_use(vulkan_context* context, vulkan_shader* shader) {
  u32 image_index = context->image_index;
  vulkan_pipeline_bind(&context->gfx_command_buffers->data[image_index],
                       VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}
void vulkan_object_shader_update_global_state(vulkan_context* context, vulkan_shader* shader) {
  u32 image_index = context->image_index;
  VkCommandBuffer cmd_buffer = context->gfx_command_buffers->data[image_index].handle;
  VkDescriptorSet global_descriptors = shader->descriptor_sets[image_index];

  vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.layout, 0,
                          1, &global_descriptors, 0, 0);

  u32 range = sizeof(global_object_uniform);
  u64 offset = 0;

  // copy data to buffer
  vulkan_buffer_load_data(context, &shader->global_uniforms_buffer, offset, range, 0,
                          &shader->global_ubo);

  VkDescriptorBufferInfo buffer_info;
  buffer_info.buffer = shader->global_uniforms_buffer.handle;
  buffer_info.offset = offset;
  buffer_info.range = range;

  VkWriteDescriptorSet descriptor_write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
  descriptor_write.dstSet = shader->descriptor_sets[image_index];
  descriptor_write.dstBinding = 0;
  descriptor_write.dstArrayElement = 0;
  descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptor_write.descriptorCount = 1;
  descriptor_write.pBufferInfo = &buffer_info;

  vkUpdateDescriptorSets(context->device.logical_device, 1, &descriptor_write, 0, 0);
}

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
#define VULKAN_QUEUES_COUNT 2
  const char* queue_names[VULKAN_QUEUES_COUNT] = {
    "GRAPHICS",
    "TRANSFER",
  };
  i32 indices[VULKAN_QUEUES_COUNT] = {
    context->device.graphics_queue_index,
    context->device.transfer_queue_index,
  };
  f32 prio_one = 1.0;
  VkDeviceQueueCreateInfo queue_create_info[2];  // HACK: make 2 queues, graphics and transfer
                                                 // graphics
  queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info[0].queueFamilyIndex = 0;
  queue_create_info[0].queueCount = 1;
  queue_create_info[0].flags = 0;
  queue_create_info[0].pNext = 0;
  queue_create_info[0].pQueuePriorities = &prio_one;
  // transfer
  queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info[1].queueFamilyIndex = 1;
  queue_create_info[1].queueCount = 1;
  queue_create_info[1].flags = 0;
  queue_create_info[1].pNext = 0;
  queue_create_info[1].pQueuePriorities = &prio_one;

  // for (int i = 0; i < 2; i++) {
  //   TRACE("Configure %s queue", queue_names[i]);
  //   queue_create_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  //   queue_create_info[i].queueFamilyIndex = indices[i];
  //   queue_create_info[i].queueCount = 1;  // make just one of them
  //   queue_create_info[i].flags = 0;
  //   queue_create_info[i].pNext = 0;
  //   f32 priority = 1.0;
  //   queue_create_info[i].pQueuePriorities = &priority;
  // }

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
  // vkGetDeviceQueue(context->device.logical_device, context->device.present_queue_index, 0,
  //                  &context->device.present_queue);
  // vkGetDeviceQueue(context->device.logical_device, context->device.compute_queue_index, 0,
  //                  &context->device.compute_queue);
  vkGetDeviceQueue(context->device.logical_device, context->device.transfer_queue_index, 0,
                   &context->device.transfer_queue);

  // create command pool for graphics queue
  VkCommandPoolCreateInfo pool_create_info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
  pool_create_info.queueFamilyIndex = context->device.graphics_queue_index;
  pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  vkCreateCommandPool(context->device.logical_device, &pool_create_info, context->allocator,
                      &context->device.gfx_command_pool);
  INFO("Created Command Pool")

  return true;
}
void vulkan_device_destroy(vulkan_context* context) {
  context->device.physical_device = 0;  // release
  // TODO: reset other memory
}

bool vulkan_device_detect_depth_format(vulkan_device* device) {
  const size_t n_candidates = 3;
  VkFormat candidates[3] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                             VK_FORMAT_D24_UNORM_S8_UINT };
  u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  for (u64 i = 0; i < n_candidates; i++) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &properties);

    if ((properties.linearTilingFeatures & flags) == flags) {
      device->depth_format = candidates[i];
      return true;
    }
    if ((properties.optimalTilingFeatures & flags) == flags) {
      device->depth_format = candidates[i];
      return true;
    }
  }
  return false;
}

void vulkan_image_view_create(vulkan_context* context, VkFormat format, vulkan_image* image,
                              VkImageAspectFlags aspect_flags) {
  VkImageViewCreateInfo view_create_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  view_create_info.image = image->handle;
  view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_create_info.format = format;
  view_create_info.subresourceRange.aspectMask = aspect_flags;

  view_create_info.subresourceRange.baseMipLevel = 0;
  view_create_info.subresourceRange.levelCount = 1;
  view_create_info.subresourceRange.baseArrayLayer = 0;
  view_create_info.subresourceRange.layerCount = 1;

  vkCreateImageView(context->device.logical_device, &view_create_info, context->allocator,
                    &image->view);
}

void vulkan_image_create(vulkan_context* context, VkImageType image_type, u32 width, u32 height,
                         VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags memory_flags, bool create_view,
                         VkImageAspectFlags aspect_flags, vulkan_image* out_image) {
  // copy params
  out_image->width = width;
  out_image->height = height;

  // create info
  VkImageCreateInfo image_create_info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
  image_create_info.imageType = image_type;
  image_create_info.extent.width = width;
  image_create_info.extent.height = height;
  image_create_info.extent.depth = 1;
  image_create_info.mipLevels = 4;
  image_create_info.arrayLayers = 1;
  image_create_info.format = format;
  image_create_info.tiling = tiling;
  image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_create_info.usage = usage;
  image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  vkCreateImage(context->device.logical_device, &image_create_info, context->allocator,
                &out_image->handle);

  VkMemoryRequirements memory_reqs;
  vkGetImageMemoryRequirements(context->device.logical_device, out_image->handle, &memory_reqs);

  i32 memory_type = -1;
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(context->device.physical_device, &memory_properties);

  for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
    // typefilter = memoryTypeBits , prop filter = memory_flags
    if (memory_reqs.memoryTypeBits & (1 << i) &&
        (memory_properties.memoryTypes[i].propertyFlags & memory_flags)) {
      memory_type = i;
      break;
    }
  }

  if (memory_type < 0) {
    ERROR_EXIT("couldnt find a suitable memory type for the image");
  }

  // allocate memory
  VkMemoryAllocateInfo memory_allocate_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  memory_allocate_info.allocationSize = memory_reqs.size;
  memory_allocate_info.memoryTypeIndex = memory_type;
  vkAllocateMemory(context->device.logical_device, &memory_allocate_info, context->allocator,
                   &out_image->memory);

  // bind memory
  // TODO: maybe bind context->device.logical_device to device at the top of the functions?
  vkBindImageMemory(context->device.logical_device, out_image->handle, out_image->memory, 0);

  if (create_view) {
    out_image->view = 0;
    vulkan_image_view_create(context, format, out_image, aspect_flags);
  }
}

// TODO: vulkan_image_destroy

void vulkan_framebuffer_create(vulkan_context* context, vulkan_renderpass* renderpass, u32 width,
                               u32 height, u32 attachment_count, VkImageView* attachments,
                               vulkan_framebuffer* out_framebuffer) {
  out_framebuffer->attachments = malloc(sizeof(VkImageView) * attachment_count);
  for (u32 i = 0; i < attachment_count; i++) {
    out_framebuffer->attachments[i] = attachments[i];
  }
  out_framebuffer->attachment_count = attachment_count;
  out_framebuffer->renderpass = renderpass;

  VkFramebufferCreateInfo framebuffer_create_info = {
    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO
  };  // TODO

  framebuffer_create_info.renderPass = renderpass->handle;
  framebuffer_create_info.attachmentCount = attachment_count;
  framebuffer_create_info.pAttachments = out_framebuffer->attachments;
  framebuffer_create_info.width = width;
  framebuffer_create_info.height = height;
  framebuffer_create_info.layers = 1;

  vkCreateFramebuffer(context->device.logical_device, &framebuffer_create_info, context->allocator,
                      &out_framebuffer->handle);
}

// TODO: vulkan_framebuffer_destroy

void vulkan_command_buffer_allocate(vulkan_context* context, VkCommandPool pool, bool is_primary,
                                    vulkan_command_buffer* out_command_buffer) {
  VkCommandBufferAllocateInfo allocate_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocate_info.commandPool = pool;
  allocate_info.level =
      is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  allocate_info.commandBufferCount = 1;
  allocate_info.pNext = 0;

  out_command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
  vkAllocateCommandBuffers(context->device.logical_device, &allocate_info,
                           &out_command_buffer->handle);
  out_command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkan_command_buffer_free(vulkan_context* context, VkCommandPool pool,
                                vulkan_command_buffer* out_command_buffer) {
  // TODO: implement freeing
}

void vulkan_command_buffer_begin(vulkan_command_buffer* command_buffer, bool is_single_use,
                                 bool is_renderpass_continue, bool is_simultaneous_use) {
  VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  begin_info.flags = 0;
  if (is_single_use) {
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  }
  // TODO: RENDER_PASS_CONTINUE_BIT & SIMULTANEOUS_USE_BIT

  begin_info.pNext = 0;
  begin_info.pInheritanceInfo = 0;
  vkBeginCommandBuffer(command_buffer->handle, &begin_info);

  command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkan_command_buffer_end(vulkan_command_buffer* command_buffer) {
  VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
  command_buffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}
void vulkan_command_buffer_update_submitted(vulkan_command_buffer* command_buffer) {
  command_buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}
void vulkan_command_buffer_reset(vulkan_command_buffer* command_buffer) {
  command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkan_command_buffer_allocate_and_begin_oneshot(vulkan_context* context, VkCommandPool pool,
                                                      vulkan_command_buffer* out_command_buffer) {
  vulkan_command_buffer_allocate(context, pool, true, out_command_buffer);
  vulkan_command_buffer_begin(out_command_buffer, true, false, false);
}

void vulkan_command_buffer_end_oneshot(vulkan_context* context, VkCommandPool pool,
                                       vulkan_command_buffer* command_buffer, VkQueue queue) {
  vulkan_command_buffer_end(command_buffer);

  // submit to queue
  VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer->handle;
  VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, 0));
  // wait for it to finish
  VK_CHECK(vkQueueWaitIdle(queue));

  vulkan_command_buffer_free(context, pool, command_buffer);
}

void vulkan_buffer_copy_to(vulkan_context* context, VkCommandPool pool, VkFence fence,
                           VkQueue queue, VkBuffer source, u64 source_offset, VkBuffer dest,
                           u64 dest_offset, u64 size) {
  vkQueueWaitIdle(queue);

  vulkan_command_buffer temp_cmd_buf;
  vulkan_command_buffer_allocate_and_begin_oneshot(context, pool, &temp_cmd_buf);

  VkBufferCopy copy_region;
  copy_region.srcOffset = source_offset;
  copy_region.dstOffset = dest_offset;
  copy_region.size = size;

  vkCmdCopyBuffer(temp_cmd_buf.handle, source, dest, 1, &copy_region);

  vulkan_command_buffer_end_oneshot(context, pool, &temp_cmd_buf, queue);
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

  u32 image_count = context->device.swapchain_support.capabilities.minImageCount;

  VkSwapchainCreateInfoKHR swapchain_create_info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  swapchain_create_info.surface = context->surface;
  swapchain_create_info.minImageCount = image_count;
  swapchain_create_info.imageFormat = out_swapchain->image_format.format;
  swapchain_create_info.imageColorSpace = out_swapchain->image_format.colorSpace;
  DEBUG("Image extent %d %d\n", swapchain_extent.width, swapchain_extent.height);
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

  TRACE("Create swapchain");
  VK_CHECK(vkCreateSwapchainKHR(context->device.logical_device, &swapchain_create_info,
                                context->allocator, &out_swapchain->handle));

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
    view_info.format = out_swapchain->image_format.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context->device.logical_device, &view_info, context->allocator,
                               &out_swapchain->views[i]));
  }

  // depth attachment
  if (!vulkan_device_detect_depth_format(&context->device)) {
    ERROR_EXIT("Failed to find a supported depth format");
  }
  vulkan_image_create(context, VK_IMAGE_TYPE_2D, swapchain_extent.width, swapchain_extent.height,
                      context->device.depth_format, VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_DEPTH_BIT,
                      &out_swapchain->depth_attachment);
  INFO("Depth attachment created");

  INFO("Swapchain created successfully");
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
    if (result == VK_SUBOPTIMAL_KHR) {
      // WARN("Swapchain suboptimal - maybe resize needed?");
    } else {
      FATAL("Failed to present swapchain iamge");
    }
  }

  // advance the current frame
  context->current_frame = (context->current_frame + 1) % swapchain->max_frames_in_flight;
}

void vulkan_renderpass_create(vulkan_context* context, vulkan_renderpass* out_renderpass,
                              vec4 render_area, vec4 clear_colour, f32 depth, u32 stencil) {
  out_renderpass->render_area = render_area;
  out_renderpass->clear_colour = clear_colour;
  out_renderpass->depth = depth;
  out_renderpass->stencil = stencil;

  // main subpass
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  // attachments
  u32 attachment_desc_count = 2;
  VkAttachmentDescription attachment_descriptions[2];

  // Colour attachment
  VkAttachmentDescription color_attachment;
  color_attachment.format = context->swapchain.image_format.format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  color_attachment.flags = 0;

  attachment_descriptions[0] = color_attachment;

  VkAttachmentReference color_attachment_reference;
  color_attachment_reference.attachment = 0;
  color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_reference;

  // Depth attachment
  VkAttachmentDescription depth_attachment;
  depth_attachment.format = context->device.depth_format;
  depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depth_attachment.flags = 0;

  attachment_descriptions[1] = depth_attachment;

  VkAttachmentReference depth_attachment_reference;
  depth_attachment_reference.attachment = 1;
  depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  subpass.pDepthStencilAttachment = &depth_attachment_reference;

  // TODO: other attachment styles

  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = 0;
  subpass.pResolveAttachments = 0;
  subpass.preserveAttachmentCount = 0;
  subpass.preserveAttachmentCount = 0;

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

  VkRenderPassCreateInfo render_pass_create_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  render_pass_create_info.attachmentCount = attachment_desc_count;
  render_pass_create_info.pAttachments = attachment_descriptions;
  render_pass_create_info.subpassCount = 1;
  render_pass_create_info.pSubpasses = &subpass;
  render_pass_create_info.dependencyCount = 1;
  render_pass_create_info.pDependencies = &dependency;
  render_pass_create_info.pNext = 0;
  render_pass_create_info.flags = 0;

  VK_CHECK(vkCreateRenderPass(context->device.logical_device, &render_pass_create_info,
                              context->allocator, &out_renderpass->handle));
}

// TODO: renderpass destroy

void vulkan_renderpass_begin(vulkan_command_buffer* command_buffer, vulkan_renderpass* renderpass,
                             VkFramebuffer framebuffer) {
  VkRenderPassBeginInfo begin_info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  begin_info.renderPass = renderpass->handle;
  begin_info.framebuffer = framebuffer;
  begin_info.renderArea.offset.x = renderpass->render_area.x;
  begin_info.renderArea.offset.y = renderpass->render_area.y;
  begin_info.renderArea.extent.width = renderpass->render_area.z;
  begin_info.renderArea.extent.height = renderpass->render_area.w;

  VkClearValue clear_values[2];
  memset(&clear_values, 0, sizeof(VkClearValue) * 2);
  clear_values[0].color.float32[0] = renderpass->clear_colour.x;
  clear_values[0].color.float32[1] = renderpass->clear_colour.y;
  clear_values[0].color.float32[2] = renderpass->clear_colour.z;
  clear_values[0].color.float32[3] = renderpass->clear_colour.w;
  clear_values[1].depthStencil.depth = renderpass->depth;
  clear_values[1].depthStencil.stencil = renderpass->stencil;

  begin_info.clearValueCount = 2;
  begin_info.pClearValues = clear_values;

  vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
  command_buffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void vulkan_renderpass_end(vulkan_command_buffer* command_buffer, vulkan_renderpass* renderpass) {
  vkCmdEndRenderPass(command_buffer->handle);
  command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

bool create_buffers(vulkan_context* context) {
  VkMemoryPropertyFlagBits mem_prop_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  const u64 vertex_buffer_size = sizeof(vertex_pos) * 1024 * 1024;
  if (!vulkan_buffer_create(context, vertex_buffer_size,
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                            mem_prop_flags, true, &context->object_vertex_buffer)) {
    ERROR("couldnt create vertex buffer");
    return false;
  }

  context->geometry_vertex_offset = 0;

  const u64 index1_buffer_size = sizeof(u32) * 1024 * 1024;
  if (!vulkan_buffer_create(context, index1_buffer_size,
                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                            mem_prop_flags, true, &context->object_index_buffer)) {
    ERROR("couldnt create vertex buffer");
    return false;
  }
  context->geometry_index_offset = 0;

  return true;
}

void create_command_buffers(renderer* ren) {
  if (!context.gfx_command_buffers) {
    context.gfx_command_buffers = vulkan_command_buffer_darray_new(context.swapchain.image_count);
  }

  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    vulkan_command_buffer_allocate(&context, context.device.gfx_command_pool, true,
                                   &context.gfx_command_buffers->data[i]);
  }
}

void upload_data_range(vulkan_context* context, VkCommandPool pool, VkFence fence, VkQueue queue,
                       vulkan_buffer* buffer, u64 offset, u64 size, void* data) {
  VkBufferUsageFlags flags =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  vulkan_buffer staging;
  vulkan_buffer_create(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, &staging);
  TRACE("HERE");
  // load data into staging buffer
  printf("Size: %ld\n", size);
  vulkan_buffer_load_data(context, &staging, 0, size, 0, data);
  TRACE("here");

  // copy
  vulkan_buffer_copy_to(context, pool, fence, queue, staging.handle, 0, buffer->handle, offset,
                        size);

  vkDestroyBuffer(context->device.logical_device, staging.handle, context->allocator);
}

void regenerate_framebuffers(renderer* ren, vulkan_swapchain* swapchain,
                             vulkan_renderpass* renderpass) {
  for (u32 i = 0; i < swapchain->image_count; i++) {
    u32 attachment_count = 2;  // one for depth, one for colour

    VkImageView attachments[2] = { swapchain->views[i], swapchain->depth_attachment.view };

    vulkan_framebuffer_create(&context, renderpass, context.framebuffer_width,
                              context.framebuffer_height, 2, attachments,
                              &swapchain->framebuffers->data[i]);
  }
}

void vulkan_fence_create(vulkan_context* context, bool create_signaled, vulkan_fence* out_fence) {
  out_fence->is_signaled = create_signaled;
  VkFenceCreateInfo fence_create_info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  if (out_fence->is_signaled) {
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  }

  vkCreateFence(context->device.logical_device, &fence_create_info, context->allocator,
                &out_fence->handle);
}

// TODO: vulkan_fence_destroy

bool vulkan_fence_wait(vulkan_context* context, vulkan_fence* fence, u64 timeout_ns) {
  if (!fence->is_signaled) {
    VkResult result =
        vkWaitForFences(context->device.logical_device, 1, &fence->handle, true, timeout_ns);
    switch (result) {
      case VK_SUCCESS:
        fence->is_signaled = true;
        return true;
      case VK_TIMEOUT:
        WARN("vk_fence_wait - Timed out");
        break;
      default:
        ERROR("vk_fence_wait - Unhanlded error type");
        break;
    }
  } else {
    return true;
  }

  return false;
}
void vulkan_fence_reset(vulkan_context* context, vulkan_fence* fence) {
  if (fence->is_signaled) {
    vkResetFences(context->device.logical_device, 1, &fence->handle);
    fence->is_signaled = false;
  }
}

bool gfx_backend_init(renderer* ren) {
  INFO("loading Vulkan backend");

  vulkan_state* internal = malloc(sizeof(vulkan_state));
  ren->backend_state = (void*)internal;

  context.allocator = 0;  // TODO: custom allocator

  context.framebuffer_width = SCR_WIDTH;
  context.framebuffer_height = SCR_HEIGHT;

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

#if defined(CDEBUG)
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
#if defined(CDEBUG)
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
#if defined(CDEBUG)
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

  // Swapchain creation
  vulkan_swapchain_create(&context, SCR_WIDTH, SCR_HEIGHT, &context.swapchain);

  // Renderpass creation
  vulkan_renderpass_create(&context, &context.main_renderpass,
                           vec4(0, 0, context.framebuffer_width, context.framebuffer_height),
                           rgba_to_vec4(COLOUR_SEA_GREEN), 1.0, 0);

  // Framebiffers creation
  context.swapchain.framebuffers = vulkan_framebuffer_darray_new(context.swapchain.image_count);
  regenerate_framebuffers(ren, &context.swapchain, &context.main_renderpass);
  INFO("Framebuffers created");

  // Command buffers creation
  create_command_buffers(ren);
  INFO("Command buffers created");

  // Sync objects
  context.image_available_semaphores =
      calloc(context.swapchain.max_frames_in_flight, sizeof(VkSemaphore));
  context.queue_complete_semaphores =
      calloc(context.swapchain.max_frames_in_flight, sizeof(VkSemaphore));
  context.in_flight_fences = calloc(context.swapchain.max_frames_in_flight, sizeof(vulkan_fence));

  for (u8 i = 0; i < context.swapchain.max_frames_in_flight; i++) {
    VkSemaphoreCreateInfo semaphore_create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    vkCreateSemaphore(context.device.logical_device, &semaphore_create_info, context.allocator,
                      &context.image_available_semaphores[i]);
    vkCreateSemaphore(context.device.logical_device, &semaphore_create_info, context.allocator,
                      &context.queue_complete_semaphores[i]);

    // create the fence in a signaled state
    vulkan_fence_create(&context, true, &context.in_flight_fences[i]);
  }

  context.images_in_flight = malloc(sizeof(vulkan_fence*) * context.swapchain.max_frames_in_flight);
  for (u8 i = 0; i < context.swapchain.max_frames_in_flight; i++) {
    context.images_in_flight[i] = 0;
  }
  INFO("Sync objects created");

  // Shader modules
  vulkan_object_shader_create(&context, &context.object_shader);
  INFO("Compiled shader modules")

  create_buffers(&context);
  INFO("Created buffers");

  // TODO: temporary test code
  const u32 vert_count = 4;
  vertex_pos verts[4] = { 0 };

  const f32 s = 10.0;

  verts[0].pos.x = 0.0 * s;
  verts[0].pos.y = -0.5 * s;

  verts[1].pos.x = 0.5 * s;
  verts[1].pos.y = 0.5 * s;

  verts[2].pos.x = 0.0 * s;
  verts[2].pos.y = 0.5 * s;

  verts[3].pos.x = 0.5 * s;
  verts[3].pos.y = -0.5 * s;

  const u32 index_count = 6;
  u32 indices[6] = { 0, 1, 2, 0, 3, 1 };

  upload_data_range(&context, context.device.gfx_command_pool, 0, context.device.graphics_queue,
                    &context.object_vertex_buffer, 0, sizeof(vertex_pos) * vert_count, verts);
  TRACE("Uploaded vertex data");
  upload_data_range(&context, context.device.gfx_command_pool, 0, context.device.graphics_queue,
                    &context.object_index_buffer, 0, sizeof(u32) * index_count, indices);
  TRACE("Uploaded index data");
  // --- End test code

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

void backend_begin_frame(renderer* ren, f32 delta_time) {
  vulkan_device* device = &context.device;

  // TODO: resize gubbins

  if (!vulkan_fence_wait(&context, &context.in_flight_fences[context.current_frame], UINT64_MAX)) {
    WARN("In-flight fence wait failure");
  }

  if (!vulkan_swapchain_acquire_next_image_index(
          &context, &context.swapchain, UINT64_MAX,
          context.image_available_semaphores[context.current_frame], 0, &context.image_index)) {
    WARN("couldnt acquire swapchain next image");
  }

  vulkan_command_buffer* command_buffer = &context.gfx_command_buffers->data[context.image_index];
  vulkan_command_buffer_reset(command_buffer);
  vulkan_command_buffer_begin(command_buffer, false, false, false);

  VkViewport viewport;
  viewport.x = 0.0;
  viewport.y = (f32)context.framebuffer_height;
  viewport.width = (f32)context.framebuffer_width;
  viewport.height = -(f32)context.framebuffer_height;
  viewport.minDepth = 0.0;
  viewport.maxDepth = 1.0;

  VkRect2D scissor;
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width = context.framebuffer_width;
  scissor.extent.height = context.framebuffer_height;

  vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
  vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

  context.main_renderpass.render_area.z = context.framebuffer_width;
  context.main_renderpass.render_area.w = context.framebuffer_height;

  vulkan_renderpass_begin(command_buffer, &context.main_renderpass,
                          context.swapchain.framebuffers->data[context.image_index].handle);

  // TODO: temp test code
  // vulkan_object_shader_use(&context, &context.object_shader);

  // VkDeviceSize offsets[1] = { 0 };
  // vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &context.object_vertex_buffer.handle,
  //                        (VkDeviceSize*)offsets);

  // vkCmdBindIndexBuffer(command_buffer->handle, context.object_index_buffer.handle, 0,
  //                      VK_INDEX_TYPE_UINT32);

  // vkCmdDrawIndexed(command_buffer->handle, 6, 1, 0, 0, 0);
  // --- End temp test code
}

void backend_end_frame(renderer* ren, f32 delta_time) {
  vulkan_command_buffer* command_buffer = &context.gfx_command_buffers->data[context.image_index];

  vulkan_renderpass_end(command_buffer, &context.main_renderpass);

  vulkan_command_buffer_end(command_buffer);

  // TODO: wait on fence - https://youtu.be/hRL71D1f3pU?si=nLJx-ZsemDBeQiQ1&t=1037

  context.images_in_flight[context.image_index] = &context.in_flight_fences[context.current_frame];

  vulkan_fence_reset(&context, &context.in_flight_fences[context.current_frame]);

  VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer->handle;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &context.queue_complete_semaphores[context.current_frame];
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &context.image_available_semaphores[context.current_frame];

  VkPipelineStageFlags flags[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  submit_info.pWaitDstStageMask = flags;

  VkResult result = vkQueueSubmit(context.device.graphics_queue, 1, &submit_info,
                                  context.in_flight_fences[context.current_frame].handle);

  if (result != VK_SUCCESS) {
    ERROR("queue submission failed. fark.");
  }

  vulkan_command_buffer_update_submitted(command_buffer);

  vulkan_swapchain_present(
      &context, &context.swapchain, context.device.graphics_queue, context.device.graphics_queue,
      context.queue_complete_semaphores[context.current_frame], context.image_index);
}

void gfx_backend_draw_frame(renderer* ren) {
  backend_begin_frame(ren, 16.0);

  static f32 z = -1.0;
  z -= 0.2;
  mat4 proj = mat4_perspective(deg_to_rad(45.0), 1000 / 1000.0, 0.1, 1000.0);
  mat4 view = mat4_translation(vec3(0, 0, z));

  gfx_backend_update_global_state(proj, view, VEC3_ZERO, vec4(1.0, 1.0, 1.0, 1.0), 0);

  backend_end_frame(ren, 16.0);
}

void gfx_backend_update_global_state(mat4 projection, mat4 view, vec3 view_pos, vec4 ambient_colour,
                                     i32 mode) {
  vulkan_command_buffer* cmd_buffer = &context.gfx_command_buffers->data[context.image_index];
  vulkan_object_shader_use(&context, &context.object_shader);

  vulkan_object_shader_update_global_state(&context, &context.object_shader);
  context.object_shader.global_ubo.projection = projection;
  context.object_shader.global_ubo.view = view;
  // TODO: other UBO properties

  // vulkan_object_shader_use(&context, &context.object_shader);

  VkDeviceSize offsets[1] = { 0 };
  vkCmdBindVertexBuffers(cmd_buffer->handle, 0, 1, &context.object_vertex_buffer.handle,
                         (VkDeviceSize*)offsets);

  vkCmdBindIndexBuffer(cmd_buffer->handle, context.object_index_buffer.handle, 0,
                       VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(cmd_buffer->handle, 6, 1, 0, 0, 0);
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