/**
 * @brief
 */

#pragma once
#include "backend_opengl.h"
#include "defines.h"
#include "ral_types.h"

typedef struct CubeMapData {
    void* top_image_data;
    void* bottom_image_data;
    void* left_image_data;
    void* right_image_data;
    void* front_image_data;
    void* back_image_data;
    u32 width, height, num_channels;
} CubeMapData;

PUB void CubeMapData_Load(const char** face_paths, int n); // should always pass n = 6 for now
PUB void CubeMapData_Free(CubeMapData* cubemap); // Frees all the image data for a cubemap
PUB TextureHandle CubeMapData_Upload(CubeMapData* cubemap);

typedef struct Skybox {
    BufferHandle vertex_buffer;
    TextureHandle texture;
    GPU_Pipeline* pipeline; // "shader"
} Skybox;

PUB Skybox Skybox_Create(const char** face_paths, int n); // should always pass n = 6 for now

PUB void Skybox_Draw(Skybox* skybox);
