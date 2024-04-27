/**
 * @file camera.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-24
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "maths_types.h"

typedef struct camera {
  vec3 position;
  vec3 front;
  vec3 up;
  f32 fov;
} camera;

/** @brief create a camera */
camera camera_create(vec3 pos, vec3 front, vec3 up, f32 fov);

/** @brief get a 4x4 transform matrix for the view and perspective projection */
void camera_view_projection(camera *c, f32 screen_height, f32 screen_width, mat4 *out_view,
                            mat4 *out_proj);

// TODO: Basic reusable camera controls
/*
Right click + move = pan
Left click = orbit camera
WASD = forward/backward/left/right
*/