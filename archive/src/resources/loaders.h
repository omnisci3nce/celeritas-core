#pragma once

#include "defines.h"
#include "render_types.h"
#include "str.h"

// --- Public API
PUB ModelHandle ModelLoad_obj(const char* path, bool invert_texture_y);
PUB ModelHandle ModelLoad_gltf(const char* path, bool invert_texture_y);

typedef struct GLTF_LoadStats {
  u32 mesh_count, material_count, vertex_count, index_count, animation_count, joint_count;
} GLTF_LoadStats;

// --- Internal
bool model_load_gltf_str(const char* file_string, const char* filepath, Str8 relative_path,
                         Model* out_model, bool invert_textures_y);
