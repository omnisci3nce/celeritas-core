/**
 * @file render_types.h
 * @brief
*/

#pragma once
#include "defines.h"
#include "ral.h"
#include "maths.h"

typedef struct Geometry {
  VertexFormat format;
  Vertex_darray* vertices;
  bool has_indices;
  u32_darray* indices;
} Geometry;

typedef struct u32_opt {
  u32 value;
  bool has_value;
} u32_opt;

typedef struct Mesh {
  BufferHandle vextex_buffer;
  BufferHandle index_buffer;
  Geometry* geometry; // NULL means it has been freed CPU-side
  bool is_uploaded; // has the data been uploaded to the GPU
} Mesh;

// --- Supported materials
typedef enum MaterialKind {
  MAT_BLINN_PHONG,
  MAT_PBR,
  MAT_PBR_PARAMS,  // uses float values to represent a surface uniformly
  MAT_COUNT
} MaterialKind;
static const char* material_kind_names[] = { "Blinn Phong", "PBR (Textures)", "PBR (Params)",
                                             "Count (This should be an error)" };

typedef struct Model {
  // meshes
  // materials
} Model;

// --- Lights
typedef struct PointLight {
  Vec3 position;
  f32 constant, linear, quadratic;
  Vec3 ambient;
  Vec3 diffuse;
  Vec3 specular;
} PointLight;

typedef struct DirectionalLight {
  Vec3 direction;
  Vec3 ambient;
  Vec3 diffuse;
  Vec3 specular;
} DirectionalLight;
