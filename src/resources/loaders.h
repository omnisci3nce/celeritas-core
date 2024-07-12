#pragma once

#include "defines.h"
#include "render_types.h"
#include "str.h"

// --- Public API
ModelHandle Model_Load_obj(const char *path, bool invert_texture_y);
ModelHandle Model_Load_gltf(const char *path, bool invert_texture_y);

// --- Internal
bool model_load_gltf_str(const char *file_string, const char *filepath, Str8 relative_path,
                         Model *out_model, bool invert_textures_y);
