/**
 * @brief
 */

#pragma once
#include "camera.h"
#include "defines.h"
#include "ral_types.h"
#include "render_types.h"

typedef struct Skybox {
    Mesh cube;
    TextureHandle texture;
    GPU_Pipeline* pipeline; // "shader"
} Skybox;

PUB Skybox Skybox_Create(const char** face_paths, int n); // should always pass n = 6 for now

PUB void Skybox_Draw(Skybox* skybox, Camera camera);

typedef struct SkyboxUniforms {
    TextureHandle cubemap;
} SkyboxUniforms;

static ShaderDataLayout Skybox_GetLayout(void* data) {
    SkyboxUniforms* d = (SkyboxUniforms*)data; // cold cast
    bool has_data = data != NULL;

    ShaderBinding b1 = {
        .label = "cubeMap",
        .vis = VISIBILITY_FRAGMENT,
        .kind = BINDING_TEXTURE,
    };

    if (has_data) {
        b1.data.texture.handle = d->cubemap;
    }
    return (ShaderDataLayout) {
        .bindings = { b1},
        .binding_count = 1
    };
}
