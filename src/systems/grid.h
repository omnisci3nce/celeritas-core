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
PUB void Grid_Init(Grid_Storage* storage);
// void Grid_Shutdown(Grid_Storage* storage);
PUB void Grid_Draw();

// --- Internal
void Grid_Execute(Grid_Storage* storage);
// typedef struct GridUniforms {} GridUniforms;
// ShaderDataLayout GridUniforms_GetLayout(void* data);