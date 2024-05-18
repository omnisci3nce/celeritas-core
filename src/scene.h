/**
 * @file scene.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-27
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include "camera.h"
#include "defines.h"
#include "maths_types.h"
#include "render_types.h"

typedef struct scene {
  // camera
  camera camera;
  // lights
  directional_light dir_light;
  point_light point_lights[4];
  size_t point_lights_count;
  // geometry
  render_entity_darray* renderables;
  // TODO: tree - transform_hierarchy
} scene;

void scene_init(scene* s);
void scene_free(scene* s);

// Simplified API - no scene pointer; gets and sets global scene

// Add/Remove objects from the scene
/* vec3 direction; */
/* vec3 ambient; */
/* vec3 diffuse; */
/* vec3 specular; */
void scene_set_dir_light(directional_light light);
void _scene_set_dir_light(vec3 ambient, vec3 diffuse, vec3 specular, vec3 direction);

void scene_add_point_light(point_light light);
void scene_add_model(model_handle model, transform3d transform);
bool scene_remove_model(model_handle model);

// Getter & Setters
void scene_set_model_transform(model_handle model, transform3d new_transform);
void scene_set_camera(vec3 pos, vec3 front);

/* // There can only be one heightmap terrain at a time right now. */
/* bool scene_add_heightmap(scene* s /\* TODO *\/); */
/* bool scene_delete_heightmap(scene* s); */

// TODO: functions to load and save scenes from disk
