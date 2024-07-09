#pragma once

// Forward declare structs - these must be defined in the backend implementation
typedef struct gpu_swapchain gpu_swapchain;
typedef struct gpu_device gpu_device;
typedef struct gpu_pipeline_layout gpu_pipeline_layout;
typedef struct gpu_pipeline gpu_pipeline;
typedef struct gpu_renderpass gpu_renderpass;
typedef struct gpu_cmd_encoder gpu_cmd_encoder;  // Recording
typedef struct gpu_cmd_buffer gpu_cmd_buffer;    // Ready for submission
typedef struct gpu_buffer gpu_buffer;
typedef struct gpu_texture gpu_texture;

typedef enum gpu_primitive_topology {
  CEL_PRIMITIVE_TOPOLOGY_POINT,
  CEL_PRIMITIVE_TOPOLOGY_LINE,
  CEL_PRIMITIVE_TOPOLOGY_LINE_STRIP,
  CEL_PRIMITIVE_TOPOLOGY_TRIANGLE,
  CEL_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
  CEL_PRIMITIVE_TOPOLOGY_COUNT
} gpu_primitive_topology;

typedef enum gpu_texture_type {
  CEL_TEXTURE_TYPE_2D,
  CEL_TEXTURE_TYPE_3D,
  CEL_TEXTURE_TYPE_2D_ARRAY,
  CEL_TEXTURE_TYPE_CUBE_MAP,
  CEL_TEXTURE_TYPE_COUNT
} gpu_texture_type;

typedef enum gpu_texture_format {
  CEL_TEXTURE_FORMAT_8_8_8_8_RGBA_UNORM,
  CEL_TEXTURE_FORMAT_DEPTH_DEFAULT,
  CEL_TEXTURE_FORMAT_COUNT
} gpu_texture_format;

// Vertex attributes
/// @strip_prefix(ATTR_)
typedef enum vertex_attrib_type {
  ATTR_F32,
  ATTR_F32x2,
  ATTR_F32x3,
  ATTR_F32x4,
  ATTR_U32,
  ATTR_U32x2,
  ATTR_U32x3,
  ATTR_U32x4,
  ATTR_I32,
  ATTR_I32x2,
  ATTR_I32x3,
  ATTR_I32x4,
} vertex_attrib_type;

typedef struct graphics_pipeline_desc {
  const char* debug_name;
  vertex_description vertex_desc;
  shader_desc vs; /** @brief Vertex shader stage */
  shader_desc fs; /** @brief Fragment shader stage */

  // Roughly equivalent to a descriptor set layout each. each layout can have multiple bindings
  // examples:
  // - uniform buffer reprensenting view projection matrix
  // - texture for shadow map
  shader_data data_layouts[MAX_SHADER_DATA_LAYOUTS];
  u32 data_layouts_count;

  // gpu_pipeline_layout* layout;
  gpu_renderpass* renderpass;

  bool wireframe;
  bool depth_test;
} graphics_pipeline_desc;

typedef struct gpu_renderpass_desc {
  bool default_framebuffer;
  bool has_color_target;
  texture_handle  color_target; // for now only support one
  bool has_depth_stencil;
  texture_handle  depth_stencil;
} gpu_renderpass_desc;