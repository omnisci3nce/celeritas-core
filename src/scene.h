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
#include "defines.h"
#include "types.h"

typedef struct scene {
  // directional_light dir_light;
  // point_light point_lights[4];
  // size_t n_point_lights;
} scene;

bool scene_add_directional_light(scene* s /* TODO */);
bool scene_add_point_light(scene* s /* TODO */);

// There can only be one heightmap terrain at a time right now.
bool scene_add_heightmap(scene* s /* TODO */);
bool scene_delete_heightmap(scene* s);

bool scene_add_model(scene *s, model_handle model);
void scene_remove_model(scene *s, model_handle model);

// TODO: functions to load and save scenes from disk