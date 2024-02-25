#pragma once

#include "defines.h"

struct core;
typedef u32 model_handle;

model_handle model_load_obj(struct core *core, const char *path, bool invert_texture_y);
model_handle model_load_gltf(struct core *core, const char *path, bool invert_texture_y);