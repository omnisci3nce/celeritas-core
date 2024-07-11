/**
 * @file camera.h
 * @brief
 */
#pragma once

#include "defines.h"
#include "maths_types.h"

typedef struct Camera {
  vec3 position;
  vec3 front;
  vec3 up;
  f32 fov;
} Camera;

/** @brief create a camera */
Camera Camera_Create(vec3 pos, vec3 front, vec3 up, f32 fov);

/**
 * @brief Get 3D camera transform matrix
 * @param out_view optionally stores just the view matrix
 * @param out_proj optionally stores just the projection matrix
 * @returns the camera's view projection matrix pre-multiplied
*/
PUB mat4 Camera_ViewProj(Camera* c, f32 lens_height, f32 lens_width, mat4* out_view, mat4* out_proj);

/** @brief Get 2D camera transform matrix */
PUB mat4 Camera_View2D(Camera* c); // TODO: 2D cameras


// TODO: Basic reusable camera controls
/*
Right click + move = pan
Left click = orbit camera
WASD = forward/backward/left/right
*/
