/**
 * @file render_types.h
 * @brief
*/

#pragma once
#include "defines.h"
#include "maths_types.h"
#include "ral.h"
#include "maths.h"
#include "ral_types.h"

// --- Handles
CORE_DEFINE_HANDLE(ModelHandle);
#define ABSENT_MODEL_HANDLE 999999999

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
  BufferHandle vertex_buffer;
  BufferHandle index_buffer;
  Geometry* geometry; // NULL means it has been freed CPU-side
  i32 material_index; // -1 => no material
  bool is_uploaded; // has the data been uploaded to the GPU
} Mesh;
#ifndef TYPED_MESH_ARRAY
KITC_DECL_TYPED_ARRAY(Mesh)
#define TYPED_MESH_ARRAY
#endif

typedef struct TextureData {
    TextureDesc description;
    void* image_data;
} TextureData;

// --- Supported materials
typedef enum MaterialKind {
  MAT_BLINN_PHONG, // NOTE: we're dropping support for this
  MAT_PBR,         // uses textures for PBR properties
  MAT_PBR_PARAMS,  // uses float values to represent a surface uniformly
  MAT_COUNT
} MaterialKind;
static const char* material_kind_names[] = { "Blinn Phong", "PBR (Textures)", "PBR (Params)",
                                             "Count (This should be an error)" };

typedef struct Material {
  char name[64];
  MaterialKind kind;
  // parameterised pbr
  Vec3 param_albedo;
  f32 param_metallic;
  f32 param_roughness;
  f32 param_ao;
  // textured pbr
  TextureHandle pbr_albedo_map;
  TextureHandle pbr_normal_map;
  bool metal_roughness_combined;
  TextureHandle pbr_metallic_map;
  TextureHandle pbr_roughness_map;
  TextureHandle pbr_ao_map;
} Material;
#ifndef TYPED_MATERIAL_ARRAY
KITC_DECL_TYPED_ARRAY(Material)
#define TYPED_MATERIAL_ARRAY
#endif

typedef struct Model {
  Str8 name;
  // meshes
  Mesh_darray* meshes;
  Material_darray* materials;
} Model;
#ifndef TYPED_MODEL_ARRAY
KITC_DECL_TYPED_ARRAY(Model)
#define TYPED_MODEL_ARRAY
#endif

// TODO: function to create a model from a single mesh (like when using primitives)

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

// ---

// A renderable 'thing'
typedef struct RenderEnt {
    Mesh* mesh;
    Material* material;
    Mat4 affine; // In the future this should be updated by the transform graph
    // Bbox_3D bounding_box;
    bool casts_shadows;
} RenderEnt;

#ifndef TYPED_RENDERENT_ARRAY
KITC_DECL_TYPED_ARRAY(RenderEnt)
#define TYPED_RENDERENT_ARRAY
#endif