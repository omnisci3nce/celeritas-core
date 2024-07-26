/**
 * @brief
 */

#pragma once
#include "defines.h"
#include "maths_types.h"
#include "ral_types.h"
#include "render_types.h"
#include "shadows.h"

typedef struct Renderer Renderer;
typedef struct GLFWwindow GLFWwindow;
typedef struct RendererConfig {
  char window_name[256];
  u32 scr_width, scr_height;
  Vec3 clear_colour;
} RendererConfig;

typedef struct RenderFlags {
  bool wireframe;
} RenderFlags;

typedef struct RenderCtx {
  Mat4 view;
  Mat4 projection;
} RenderCtx;

// --- Lifecycle

PUB bool Renderer_Init(RendererConfig config, Renderer* renderer, GLFWwindow** out_window,
                       GLFWwindow* optional_window);
PUB void Renderer_Shutdown(Renderer* renderer);
PUB size_t Renderer_GetMemReqs();
void Render_WindowSizeChanged(GLFWwindow* window, i32 new_width, i32 new_height);

// internal init functions
void DefaultPipelinesInit(Renderer* renderer);

// NOTE: All of these functions grab the Renderer instance off the global Core
PUB void Render_FrameBegin(Renderer* renderer);
PUB void Render_FrameEnd(Renderer* renderer);

/** @brief  */
PUB void Render_RenderEntities(RenderEnt* entities, size_t entity_count);

// TODO: Render_FrameDraw(); - this will

// --- Resources

PUB TextureData TextureDataLoad(const char* path, bool invert_y);
PUB void TextureUpload(TextureHandle handle, size_t n_bytes, const void* data);
PUB TextureHandle TextureLoadFromFile(const char* path);
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

// --- Getters (not in love with this but I'm finding keeping Renderer internals private to be okay)
arena* GetRenderFrameArena(Renderer* r);

typedef struct RenderScene RenderScene;
typedef struct Shadow_Storage Shadow_Storage;
typedef struct Terrain_Storage Terrain_Storage;

RenderScene* Render_GetScene();
Shadow_Storage* Render_GetShadowStorage();
Terrain_Storage* Render_GetTerrainStorage();