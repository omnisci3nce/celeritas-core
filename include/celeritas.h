/**
 * @file celeritas.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

/* The Goal of this file is to test ocaml-bindgen on it to start moving development over into OCaml */

#include <stdbool.h>
#include <stdint.h>


// Forward Declarations
typedef struct core core;

// Handles
typedef uint32_t model_handle;

// Maths
typedef struct vec2 { float x, y; } vec2;
typedef struct vec3 { float x, y, z; } vec3;
typedef struct vec4 { float x, y, z, w; } vec4;
typedef struct mat4 { float data[16]; } mat4;
typedef struct transform3d { vec3 translation; vec4 rotation; float scale; } transform3d;

// Lifecycle functions
void core_bringup();
void core_shutdown();
bool should_window_close();

void render_frame_begin();
void render_frame_draw();
void render_frame_end();

// Assets
model_handle model_load(const char* filepath);

// Rendering
typedef struct render_entity {
  model_handle model;
  // TODO: material
  transform3d transform;
} render_entity;

// Scene
typedef struct directional_light {} directional_light;
typedef struct point_light {} point_light;
void scene_add_dir_light(directional_light light);
void scene_add_point_light(directional_light light);
void scene_add_model(model_handle model, transform3d transform);
bool scene_remove_model(model_handle model);

void scene_set_model_transform(model_handle model, transform3d new_transform);
void scene_set_camera(vec3 pos, vec3 front);

// Immediate mode drawing

// Input