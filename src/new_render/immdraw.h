/**
 * @brief Immediate-mode drawing APIs
 */

#pragma once
#include "defines.h"
#include "maths_types.h"

typedef struct Immdraw_Storage {
} Immdraw_Storage;

// --- Public API

PUB void Immdraw_Init(Immdraw_Storage* storage);
PUB void Immdraw_Shutdown(Immdraw_Storage* storage);

// These functions cause a pipeline switch and so aren't optimised for performance
PUB void Immdraw_Plane(Transform tf, Vec4 colour, bool wireframe);
PUB void Immdraw_Cuboid(Transform tf, Vec4 colour, bool wireframe);
PUB void Immdraw_Sphere(Transform tf, f32 size, Vec4 colour, bool wireframe);
PUB void Immdraw_TransformGizmo(Transform tf, f32 size);
