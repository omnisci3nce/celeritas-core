#include "str.h"
#define CEL_PLATFORM_LINUX

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
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

typedef struct vulkan_context {
  VkInstance instance;
  VkAllocationCallbacks* allocator;
} vulkan_context;

static vulkan_context context;

/** @brief Internal backend state */
typedef struct vulkan_state {
} vulkan_state;

KITC_DECL_TYPED_ARRAY(VkLayerProperties)

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

  INFO("Vulkan renderer initialisation succeeded");
  return true;
}

void gfx_backend_shutdown(renderer* ren) {}

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

#endif