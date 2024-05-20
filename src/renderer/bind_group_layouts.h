/**
 * @file bind_group_layouts.h
 * @author your name (you@domain.com)
 * @brief Common bindgroups (descriptor set layouts)
 * @version 0.1
 * @date 2024-04-28
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include "defines.h"
#include "maths_types.h"

// Three major sets

// 1. Scene / Global
typedef struct bg_globals {
  mat4 view;
  mat4 projection;
  f32 total_time;
  f32 delta_time;
} bg_globals;

// 2. Material (once per object)

// 3. Per draw call
typedef struct bg_model {
  mat4 model;
} bg_model;
