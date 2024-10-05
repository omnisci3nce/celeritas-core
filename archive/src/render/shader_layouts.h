#pragma once
#include "maths_types.h"
#include "ral_types.h"

/** @brief shader layout for camera matrices */
typedef struct Binding_Camera {
  Mat4 view;
  Mat4 projection;
  Vec4 viewPos;
} Binding_Camera;

typedef struct Binding_Model {
  Mat4 model;
} Binding_Model;

/** @brief data that is handy to have in any shader */
typedef struct Binding_Globals {
} Binding_Globals;

typedef struct pbr_point_light {
  Vec3 pos;
  f32 pad;
  Vec3 color;
  f32 pad2;
} pbr_point_light;

typedef struct Binding_Lights {
  pbr_point_light pointLights[4];
} Binding_Lights;

static ShaderDataLayout Binding_Camera_GetLayout(void* data) {
  Binding_Camera* d = data;
  bool has_data = data != NULL;

  ShaderBinding b1 = { .label = "Camera",
                       .kind = BINDING_BYTES,
                       .data.bytes = { .size = sizeof(Binding_Camera) } };
  if (has_data) {
    b1.data.bytes.data = d;
  }
  return (ShaderDataLayout){ .bindings = { b1 }, .binding_count = 1 };
}

static ShaderDataLayout Binding_Model_GetLayout(void* data) {
  Binding_Model* d = data;
  bool has_data = data != NULL;

  ShaderBinding b1 = { .label = "Model",
                       .kind = BINDING_BYTES,
                       .vis = VISIBILITY_VERTEX,
                       .data.bytes = { .size = sizeof(Binding_Model) } };
  if (has_data) {
    b1.data.bytes.data = d;
  }
  return (ShaderDataLayout){ .bindings = { b1 }, .binding_count = 1 };
}

static ShaderDataLayout Binding_Lights_GetLayout(void* data) {
  Binding_Lights* d = data;
  bool has_data = data != NULL;

  ShaderBinding b1 = { .label = "Lights",
                       .kind = BINDING_BYTES,
                       .vis = VISIBILITY_FRAGMENT,
                       .data.bytes = { .size = sizeof(Binding_Lights) } };
  if (has_data) {
    b1.data.bytes.data = d;
  }
  return (ShaderDataLayout){ .bindings = { b1 }, .binding_count = 1 };
}
