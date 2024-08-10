/**
 * @brief
 */

#pragma once
#include "defines.h"
#include "grid.h"
#include "maths_types.h"
#include "ral_types.h"
#include "render_types.h"
#include "shadows.h"
#include "camera.h"

typedef struct Renderer Renderer;
typedef struct GLFWwindow GLFWwindow;
typedef struct RendererConfig {
  const char* window_name;
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

/** @brief Holds globally bound data for rendering a scene. Typically held by the renderer.
 *         Whenever you call draw functions you can think of this as an implicit parameter. */
typedef struct RenderScene {
  Camera camera;
  DirectionalLight sun;
} RenderScene;

PUB void SetCamera(Camera camera);
PUB void SetMainLight(DirectionalLight light);

// #define MESH_GET(h) (Mesh_pool_get(g_core.renderer->meshes, h))
// #define MATERIAL_GET(h) (Material_pool_get(g_core.renderer->material, h))

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
Mesh* Mesh_Get(MeshHandle handle);
void Geometry_Destroy(Geometry* geometry);
MeshHandle Mesh_Insert(Mesh* mesh);
MaterialHandle Material_Insert(Material* material);

/** @brief gets render entities from a model and pushes them into a dynamic array for rendering */
size_t ModelExtractRenderEnts(RenderEnt_darray* entities, ModelHandle model_handle, Mat4 affine, RenderEntityFlags flags);

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
Grid_Storage* Render_GetGridStorage();
TextureHandle Render_GetWhiteTexture();
arena* Render_GetFrameArena();
Mesh_pool* Render_GetMeshPool();
Material_pool* Render_GetMaterialPool();

// --- Setters
void Render_SetRenderMode(RenderMode mode);

// -------------------------------------------------

// Frame lifecycle on CPU

// 1. extract
// 2. culling
// 3. render
// 4. dispatch (combined with render for now)

// typedef struct Cull_Result {
//   u64 n_visible_objects;
//   u64 n_culled_objects;
//   u32* visible_ent_indices;  // allocated on frame arena
//   size_t index_count;
// } Cull_Result;

// // everything that can be in the world, knows how to extract rendering data
// typedef void (*ExtractRenderData)(void* world_data);

// typedef struct Renderer Renderer;

// /** @brief Produces a smaller set of only those meshes visible in the camera frustum on the CPU */
// Cull_Result Frame_Cull(Renderer* ren, RenderEnt* entities, size_t entity_count, Camera* camera);

// Cull_Result Frame_Cull(Renderer* ren, RenderEnt* entities, size_t entity_count, Camera* camera) {
//   // TODO: u32 chunk_count = Tpool_GetNumWorkers();

//   arena* frame_arena = GetRenderFrameArena(ren);

//   Cull_Result result = { 0 };
//   result.visible_ent_indices = arena_alloc(
//       frame_arena, sizeof(u32) * entity_count);  // make space for if all ents are visible

//   assert((result.n_visible_objects + result.n_culled_objects == entity_count));
//   return result;
// }
