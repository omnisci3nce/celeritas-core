#pragma once

#include "defines.h"
#include "maths_types.h"

// TODO: swap to position + quaternion

typedef struct Camera {
  Vec3 position;
  Vec3 front;
  Vec3 up;
  f32 fov;
} Camera;

/** @brief create a camera */
PUB Camera Camera_Create(Vec3 pos, Vec3 front, Vec3 up, f32 fov);

/**
 * @brief Get 3D camera transform matrix
 * @param out_view optionally stores just the view matrix
 * @param out_proj optionally stores just the projection matrix
 * @returns the camera's view projection matrix pre-multiplied
 */
PUB Mat4 Camera_ViewProj(Camera* c, f32 lens_height, f32 lens_width, Mat4* out_view,
                         Mat4* out_proj);

/** @brief Get 2D camera transform matrix */
PUB Mat4 Camera_View2D(Camera* c);  // TODO: 2D cameras

struct Input_State;

PUB void FlyCamera_Update(Camera* camera);

// TODO: (HIGH) Basic reusable camera controls
/*
Right click + move = pan
Left click = orbit camera
WASD = forward/backward/left/right
*/
