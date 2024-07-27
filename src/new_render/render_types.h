/**
 * @file render_types.h
 * @brief
 */

#pragma once
#include "defines.h"
#include "maths_types.h"
#include "mem.h"
#include "ral_types.h"

// --- Handles
CORE_DEFINE_HANDLE(ModelHandle);
CORE_DEFINE_HANDLE(MaterialHandle);
CORE_DEFINE_HANDLE(MeshHandle);
#define INVALID_MODEL_HANDLE ((ModelHandle){ .raw = 9999991 })
#define INVALID_MATERIAL_HANDLE ((MaterialHandle){ .raw = 9999992 })
#define INVALID_MESH_HANDLE ((MeshHandle){ .raw = 9999993 })

typedef struct Geometry {
  VertexFormat format;
  Vertex_darray* vertices;
  u32_darray* indices;
  bool has_indices;
  size_t index_count;
} Geometry;

typedef struct u32_opt {
  u32 value;
  bool has_value;
} u32_opt;

typedef struct Mesh {
  BufferHandle vertex_buffer;
  BufferHandle index_buffer;
  Geometry geometry;  // NULL means it has been freed CPU-side
  // i32 material_index;  // -1 => no material
  MaterialHandle material;
  bool is_uploaded;  // has the data been uploaded to the GPU
} Mesh;
#ifndef TYPED_MESH_CONTAINERS
KITC_DECL_TYPED_ARRAY(Mesh)
TYPED_POOL(Mesh, Mesh)
#define TYPED_MESH_CONTAINERS
#endif

typedef struct TextureData {
  TextureDesc description;
  void* image_data;
} TextureData;

// --- Supported materials
typedef enum MaterialKind {
  MAT_BLINN_PHONG,  // NOTE: we're dropping support for this
  MAT_PBR,          // uses textures for PBR properties
  MAT_PBR_PARAMS,   // uses float values to represent a surface uniformly
  MAT_COUNT
} MaterialKind;
static const char* material_kind_names[] = { "Blinn Phong", "PBR (Textures)", "PBR (Params)",
                                             "Count (This should be an error)" };

/**
 * @brief
 * @note based on https://google.github.io/filament/Filament.html#materialsystem/standardmodel
 */
typedef struct Material {
  char name[64];
  MaterialKind kind;  // at the moment all materials are PBR materials
  Vec3 base_colour;   // linear RGB {0,0,0} to {1,1,1}
  f32 metallic;
  f32 roughness;
  f32 ambient_occlusion;
  TextureHandle albedo_map;
  TextureHandle normal_map;
  TextureHandle metallic_roughness_map;
  TextureHandle ambient_occlusion_map;
} Material;

#ifndef TYPED_MATERIAL_CONTAINERS
KITC_DECL_TYPED_ARRAY(Material)
TYPED_POOL(Material, Material)
#define TYPED_MATERIAL_CONTAINERS
#endif

/** @brief Convenient wrapper around a number of meshes each with a material */
typedef struct Model {
  Str8 name;
  MeshHandle* meshes;
  size_t mesh_count;
  MaterialHandle* materials;
  size_t material_count;
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

typedef enum RenderEntityFlag {
  REND_ENT_CASTS_SHADOWS = 1 << 0,
  REND_ENT_VISIBLE = 1 << 1,
} RenderEntityFlag;
typedef u32 RenderEntityFlags;

/** @brief A renderable 'thing' */
typedef struct RenderEnt {
  MeshHandle mesh;
  MaterialHandle material;
  Mat4 affine;  // In the future this should be updated by the transform graph
  Bbox_3D bounding_box;
  RenderEntityFlags flags;
} RenderEnt;

#ifndef TYPED_RENDERENT_ARRAY
KITC_DECL_TYPED_ARRAY(RenderEnt)
#define TYPED_RENDERENT_ARRAY
#endif