/**
 * @file render_types.h
 * @author Omniscient
 * @brief Type definitions for the majority of data required by the renderer system
 * @date 2024-02-24
 *
 */
#pragma once

#include "darray.h"
#include "maths_types.h"

struct GLFWwindow;

#ifndef RESOURCE_HANDLE_DEFS
CORE_DEFINE_HANDLE(texture_handle);
#define RESOURCE_HANDLE_DEFS
#endif

/* @brief Opaque wrapper around a shader program */
typedef struct shader {
  u32 program_id;
} shader;

/** @brief configuration passed to the renderer at init time */
typedef struct renderer_config {
  char window_name[256];
  u32 scr_width, scr_height;
  vec3 clear_colour; /** colour that the screen gets cleared to every frame */
} renderer_config;

typedef struct renderer {
  struct GLFWwindow *window; /** Currently all platforms use GLFW*/
  void *backend_state;       /** Graphics API-specific state */
  renderer_config config;
} renderer;

/** @brief Vertex format for a static mesh */
typedef struct vertex {
  vec3 position;
  vec3 normal;
  vec2 uv;
} vertex;

#ifndef TYPED_VERTEX_ARRAY
KITC_DECL_TYPED_ARRAY(vertex)  // creates "vertex_darray"
#define TYPED_VERTEX_ARRAY
#endif

// --- Models & Meshes

typedef struct mesh {
  vertex_darray *vertices;
  u32 vertex_size; /** size in bytes of each vertex including necessary padding */
  bool has_indices;
  u32 *indices;
  u32 indices_len;
  size_t material_index;
  u32 vbo, vao; /** OpenGL data */
} mesh;

#ifndef TYPED_MESH_ARRAY
KITC_DECL_TYPED_ARRAY(mesh)  // creates "mesh_darray"
#define TYPED_MESH_ARRAY
#endif

typedef struct model {
  char name[256];
} model;

#ifndef TYPED_MODEL_ARRAY
KITC_DECL_TYPED_ARRAY(model)  // creates "model_darray"
#define TYPED_MODEL_ARRAY
#endif