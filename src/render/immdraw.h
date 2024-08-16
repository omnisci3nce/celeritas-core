/**
 * @brief Immediate-mode drawing APIs
 */

#pragma once
#include "defines.h"
#include "maths_types.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render_types.h"

typedef struct Immdraw_Storage {
  Mesh plane;
  Mesh cube;
  Mesh sphere;
  Mesh cone;
  Mesh bbox;
  GPU_Pipeline* colour_pipeline; /** @brief Pipeline for drawing geometry that has vertex colours */
} Immdraw_Storage;

typedef struct ImmediateUniforms {
  Mat4 model;
  Vec4 colour;
} ImmediateUniforms;

// --- Public API

PUB void Immdraw_Init(Immdraw_Storage* storage);
PUB void Immdraw_Shutdown(Immdraw_Storage* storage);

// These functions cause a pipeline switch and so aren't optimised for performance
PUB void Immdraw_Plane(Transform tf, Vec4 colour, bool wireframe);
PUB void Immdraw_Cuboid(Transform tf, Vec4 colour, bool wireframe);
PUB void Immdraw_Cone(Transform tf, Vec4 colour, bool wireframe);
PUB void Immdraw_Sphere(Transform tf, Vec4 colour, bool wireframe);
PUB void Immdraw_Bbox(Transform tf, Vec4 colour, bool wireframe);

PUB void Immdraw_TransformGizmo(Transform tf, f32 size);

// --- Internal

void Immdraw_Primitive(Transform tf, PrimitiveTopology topology, f32 size, Vec4 colour,
                       bool wireframe, Mesh mesh);

Mesh GenBboxMesh();

static ShaderDataLayout ImmediateUniforms_GetLayout(void* data) {
  ImmediateUniforms* d = (ImmediateUniforms*)data;
  bool has_data = data != NULL;

  ShaderBinding b1 = { .label = "ImmUniforms",
                       .kind = BINDING_BYTES,
                       // .vis = VISIBILITY_VERTEX,
                       .data.bytes.size = sizeof(ImmediateUniforms) };

  if (has_data) {
    b1.data.bytes.data = d;
  }

  return (ShaderDataLayout){ .bindings = { b1 }, .binding_count = 1 };
}
