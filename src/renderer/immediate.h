#pragma once

#include "geometry.h"
#include "maths_types.h"

typedef struct immdraw_system immdraw_system;

bool immdraw_system_init(immdraw_system* state);
void immdraw_system_shutdown(immdraw_system* state);
void immdraw_system_render(immdraw_system* state);

// 3. SIMA (simplified immediate mode api) / render.h
//      - dont need to worry about uploading mesh data
//      - very useful for debugging
void immdraw_plane(vec3 pos, quat rotation, f32 u_scale, f32 v_scale, vec4 colour);
void immdraw_cuboid(vec3 pos, quat rotation, f32x3 extents, vec4 colour);
void immdraw_sphere(vec3 pos, f32 radius, vec4 colour);

void immdraw_camera_frustum();
