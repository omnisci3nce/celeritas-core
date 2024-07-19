/**
 * @file render_scene.h
 * @brief
 */
#pragma once
#include "camera.h"
#include "defines.h"
#include "render_types.h"

/** @brief Holds globally bound data for rendering a scene. Typically held by the renderer.
 *         Whenever you call draw functions you can think of this as an implicit parameter. */
typedef struct RenderScene {
  Camera camera;
  DirectionalLight sun;
} RenderScene;

// --- Public APIs

PUB void SetCamera(Camera camera);
PUB void SetMainLight(DirectionalLight light);
