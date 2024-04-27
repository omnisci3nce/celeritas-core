#pragma once

#include "cleanroom/ral.h"
#include "cleanroom/backend_vulkan.h"

typedef struct renderer2 {
  void* backend_state;
  gpu_device* device;
  gpu_pipeline* static_opaque_pipeline;
} renderer2;

// mesh
// model
// material