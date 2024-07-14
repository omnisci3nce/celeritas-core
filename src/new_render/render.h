/**
 * @brief
 */

#pragma once
#include "defines.h"
#include "maths_types.h"
#include "ral_types.h"
#include "render_types.h"

typedef struct Renderer Renderer;
typedef struct RendererConfig {
    char window_name[256];
    u32 scr_width, scr_height;
    Vec3 clear_colour;
} RendererConfig;

typedef struct RenderCtx {
    Mat4 view;
    Mat4 projection;
} RenderCtx;

// --- Lifecycle

PUB bool Renderer_Init(RendererConfig config, Renderer* renderer);
PUB void Renderer_Shutdown(Renderer* renderer);
PUB size_t Renderer_GetMemReqs();

// internal init functions
void DefaultPipelinesInit(Renderer* renderer);

// NOTE: All of these functions grab the Renderer instance off the global Core
PUB void Render_FrameBegin(Renderer* renderer);
PUB void Render_FrameEnd(Renderer* renderer);

/** @brief  */
PUB void Render_RenderEntities(RenderEnt* entities, size_t entity_count);

// TODO: Render_FrameDraw(); - this will

// --- Resources

PUB TextureHandle TextureUpload();
PUB ModelHandle ModelLoad(const char* debug_name, const char* filepath);

// --- Rendering Data

PUB Mesh Mesh_Create(Geometry* geometry, bool free_on_upload);
PUB void Mesh_Delete(Mesh* mesh);
void Geometry_Destroy(Geometry* geometry);

// --- Drawing

// NOTE: These functions use the globally bound camera in RenderScene
PUB void DrawMesh(Mesh* mesh, Material* material, Mat4 model);

/** @brief the renderer does some internal bookkeeping for terrain so we use the terrain
           stored on the Renderer rather than accept it as a parameter */
PUB void Render_DrawTerrain();
