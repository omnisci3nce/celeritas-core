#pragma once

#include "maths_types.h"

// 3. SIMA (simplified immediate mode api) / render.h
//      - dont need to worry about uploading mesh data
//      - very useful for debugging
void imm_draw_cuboid();
void imm_draw_sphere(vec3 pos, f32 radius, vec4 colour);
void imm_draw_camera_frustum();
static void imm_draw_model(
    const char* model_filepath);  // tracks internally whether the model is loaded

static void imm_draw_model(const char* model_filepath) {
  // check that model is loaded
  // if not loaded, load model and upload to gpu - LRU cache for models
  // else submit draw call
}