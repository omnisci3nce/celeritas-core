#pragma once

#include "ral_impl.h"
#include "ral_types.h"

typedef struct Grid_Storage {
  GPU_Renderpass* renderpass;
  GPU_Pipeline* pipeline;
  BufferHandle plane_vertices;
  BufferHandle plane_indices;
} Grid_Storage;

// --- Public API
void Grid_Init(Grid_Storage* storage);
// void Grid_Shutdown(Grid_Storage* storage);
void Grid_Draw(Grid_Storage* storage);

// --- Internal
// typedef struct GridUniforms {} GridUniforms;
// ShaderDataLayout GridUniforms_GetLayout(void* data);