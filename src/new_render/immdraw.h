/**
 * @brief Immediate-mode drawing APIs
 */

#pragma once
#include "defines.h"
#include "maths_types.h"

// --- Public API


void Immdraw_Cuboid(Transform tf);
void Immdraw_Sphere(Transform tf, f32 size);
void Immdraw_TransformGizmo(Transform tf, f32 size);
