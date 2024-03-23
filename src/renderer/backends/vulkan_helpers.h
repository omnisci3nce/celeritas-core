#pragma once

#include <assert.h>
#include <vulkan/vulkan.h>

#include "darray.h"
#include "defines.h"

DECL_TYPED_ARRAY(const char*, cstr)

static void plat_get_required_extension_names(cstr_darray* extensions) {
#ifdef CEL_PLATFORM_LINUX
  cstr_darray_push(extensions, "VK_KHR_xcb_surface");
#endif
}

// TODO(omni): port to using internal assert functions
#define VK_CHECK(vulkan_expr) \
  { assert(vulkan_expr == VK_SUCCESS); }