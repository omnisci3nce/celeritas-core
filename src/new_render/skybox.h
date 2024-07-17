/**
 * @brief
 */

#pragma once
#include "backend_opengl.h"
#include "defines.h"
#include "ral_types.h"

typedef struct Skybox {
    BufferHandle vertex_buffer;
    TextureHandle texture;
    GPU_Pipeline* pipeline; // "shader"
} Skybox;

PUB Skybox Skybox_Create(const char** face_paths, int n); // should always pass n = 6 for now

PUB void Skybox_Draw(Skybox* skybox);

typedef struct SkyboxUniforms {
    Vec3 in_pos;
    TextureHandle cubemap;
} SkyboxUniforms;

static ShaderDataLayout Skybox_GetLayout(void* data) {
    SkyboxUniforms* d = (SkyboxUniforms*)data; // cold cast
    bool has_data = data != NULL;

    ShaderBinding b1 = {
        .label = "In",
        .vis = VISIBILITY_VERTEX,
        .kind = BINDING_BYTES,
        .data = {.bytes = {.size = sizeof(Vec3)}}
    };

    ShaderBinding b2 = {
        .label = "cubemap",
        .vis = VISIBILITY_FRAGMENT,
        .kind = BINDING_SAMPLER,
    };

    if (has_data) {
        b1.data.bytes.data = &d->in_pos;
        b2.data.texture.handle = d->cubemap;
    }
    return (ShaderDataLayout) {
        .bindings = { b1, b2},
        .binding_count = 2
    };
}