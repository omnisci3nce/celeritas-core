/**
 * @file draw.h
 * @brief
 */
#pragma once
#include "defines.h"
#include "maths_types.h"
#include "render_types.h"

// --- Public APIs

PUB void EncodeDrawModel(Handle model, Mat4 transform);
PUB void EncodeDrawMesh(Mesh* mesh, Material* material, Mat4 affine);
