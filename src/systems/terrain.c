/**
 * @brief
 */
#include "terrain.h"
#include <assert.h>
#include "file.h"
#include "glad/glad.h"
#include "log.h"
#include "maths.h"
#include "mem.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render.h"
#include "render_scene.h"
#include "shader_layouts.h"
#include "str.h"

#define TERRAIN_GRID_U 64
#define TERRAIN_GRID_V 64

bool Terrain_Init(Terrain_Storage* storage) {
  storage->grid_dimensions = u32x2(TERRAIN_GRID_U, TERRAIN_GRID_V);
  storage->hmap_loaded = false;

  GPU_RenderpassDesc rpass_desc = {
    .default_framebuffer = true,
  };
  storage->hmap_renderpass = GPU_Renderpass_Create(rpass_desc);

  VertexDescription builder = { .debug_label = "pos only" };
  VertexDesc_AddAttr(&builder, "inPosition", ATTR_F32x3);
  builder.use_full_vertex_size = true;

  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  Str8 vert_path = str8("assets/shaders/terrain.vert");
  Str8 frag_path = str8("assets/shaders/terrain.frag");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  ShaderData camera_data = { .get_layout = &Binding_Camera_GetLayout };

  GraphicsPipelineDesc pipeline_desc = {
    .debug_name = "terrain rendering pipeline",
    .vertex_desc = builder,
    .data_layouts = { camera_data },
    .data_layouts_count = 1,
    .vs = {
      .debug_name = "terrain vertex shader",
      .filepath = vert_path,
      .code = vertex_shader.contents,
    },
    .fs = {
      .debug_name = "terrain fragment shader",
      .filepath = frag_path,
      .code = fragment_shader.contents,
    },
    .wireframe = true,
  };
  storage->hmap_pipeline = GPU_GraphicsPipeline_Create(pipeline_desc, storage->hmap_renderpass);

  return true;
}

void Terrain_Shutdown(Terrain_Storage* storage) {}

void Terrain_LoadHeightmap(Terrain_Storage* storage, Heightmap hmap, bool free_on_upload) {
  // If there's a current one we will delete it and reallocate buffers
  if (storage->hmap_loaded) {
    GPU_BufferDestroy(storage->vertex_buffer);
    GPU_BufferDestroy(storage->index_buffer);
  }

  size_t num_vertices = storage->grid_dimensions.x * storage->grid_dimensions.y;
  storage->num_vertices = num_vertices;

  Vertex_darray* vertices = Vertex_darray_new(num_vertices);
  u32 index = 0;
  for (u32 i = 0; i < storage->grid_dimensions.x; i++) {
    for (u32 j = 0; j < storage->grid_dimensions.y; j++) {
      assert(index < num_vertices);
      Vertex v = { .pos_only.position = vec3_create(i, 0.0, j) };
      Vertex_darray_push(vertices, v);
      index++;
    }
  }

  BufferHandle vertices_handle = GPU_BufferCreate(num_vertices * sizeof(Vertex), BUFFER_VERTEX,
                                                  BUFFER_FLAG_GPU, vertices->data);

  storage->vertex_buffer = vertices_handle;
}

Heightmap Heightmap_FromImage(Str8 filepath) {
  size_t max_size = MB(16);
  arena arena = arena_create(malloc(max_size), max_size);
  str8_opt maybe_file = str8_from_file(&arena, filepath);

  assert(maybe_file.has_value);

  TextureData hmap_tex = TextureDataLoad(Str8_to_cstr(&arena, filepath), false);

  arena_free_storage(&arena);

  return (Heightmap){
    .pixel_dimensions = hmap_tex.description.extents,
    .filepath = filepath,
    .image_data = hmap_tex.image_data,
    .is_uploaded = false,
  };
}

void Terrain_Draw(Terrain_Storage* storage) {
  GPU_CmdEncoder* enc = GPU_GetDefaultEncoder();
  GPU_EncodeBindPipeline(enc, storage->hmap_pipeline);
  RenderScene* scene = Render_GetScene();

  Mat4 view, proj;
  u32x2 dimensions = GPU_Swapchain_GetDimensions();
  Camera_ViewProj(&scene->camera, (f32)dimensions.x, (f32)dimensions.y, &view, &proj);
  Binding_Camera camera_data = { .view = view,
                                 .projection = proj,
                                 .viewPos = vec4(scene->camera.position.x, scene->camera.position.y,
                                                 scene->camera.position.z, 1.0) };
  GPU_EncodeBindShaderData(
      enc, 0, (ShaderData){ .data = &camera_data, .get_layout = &Binding_Camera_GetLayout });

  GPU_EncodeSetVertexBuffer(enc, storage->vertex_buffer);

  // GPU_EncodeDraw(enc, storage->num_vertices);
  glDrawArrays(GL_POINTS, 0, storage->num_vertices);
}
