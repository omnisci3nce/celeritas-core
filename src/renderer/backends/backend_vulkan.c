#define CEL_PLATFORM_LINUX

#include "defines.h"
#include "file.h"
#include "log.h"
#include "maths_types.h"
#include "render_types.h"

#include <stdlib.h>

#if CEL_REND_BACKEND_VULKAN

#include <glad/glad.h>

#include <glfw3.h>

typedef struct vulkan_context vulkan_context;
static vulkan_context context;

/** @brief Internal backend state */
typedef struct vulkan_state {
} vulkan_state;