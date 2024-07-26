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
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render.h"
#include "render_scene.h"
#include "shader_layouts.h"
#include "str.h"

#define TERRAIN_GRID_U 505
#define TERRAIN_GRID_V 505

bool Terrain_Init(Terrain_Storage* storage) {
  storage->grid_dimensions = u32x2(TERRAIN_GRID_U, TERRAIN_GRID_V);
  storage->hmap_loaded = false;

  GPU_RenderpassDesc rpass_desc = {
    .default_framebuffer = true,
  };
  storage->hmap_renderpass = GPU_Renderpass_Create(rpass_desc);

  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  Str8 vert_path = str8("assets/shaders/terrain.vert");
  Str8 frag_path = str8("assets/shaders/terrain.frag");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  ShaderData camera_data = { .get_layout = &Binding_Camera_GetLayout };
  ShaderData terrain_data = { .get_layout = &TerrainUniforms_GetLayout };

  GraphicsPipelineDesc pipeline_desc = {
    .debug_name = "terrain rendering pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = { camera_data, terrain_data },
    .data_layouts_count = 2,
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
    .wireframe = false,
  };
  storage->hmap_pipeline = GPU_GraphicsPipeline_Create(pipeline_desc, storage->hmap_renderpass);

  storage->texture = TextureLoadFromFile("assets/demo/textures/grass2.png");

  return true;
}

void Terrain_Shutdown(Terrain_Storage* storage) {}

void Terrain_LoadHeightmap(Terrain_Storage* storage, Heightmap hmap, f32 grid_scale,
                           bool free_on_upload) {
  // If there's a current one we will delete it and reallocate buffers
  if (storage->hmap_loaded) {
    GPU_BufferDestroy(storage->vertex_buffer);
    GPU_BufferDestroy(storage->index_buffer);
  }

  u32 width = hmap.pixel_dimensions.x;
  u32 height = hmap.pixel_dimensions.y;
  storage->grid_scale = grid_scale;

  size_t num_vertices = storage->grid_dimensions.x * storage->grid_dimensions.y;
  storage->num_vertices = num_vertices;

  Vertex_darray* vertices = Vertex_darray_new(num_vertices);
  u32 index = 0;
  for (u32 i = 0; i < storage->grid_dimensions.x; i++) {
    for (u32 j = 0; j < storage->grid_dimensions.y; j++) {
      size_t position = j * storage->grid_dimensions.x + i;
      u8* bytes = hmap.image_data;
      u8 channel = bytes[position];
      float value = (float)channel / 255.0;
      // printf("(%d, %d) %d : %f \n", i, j, channel, value);

      assert(index < num_vertices);
      f32 height = Heightmap_HeightXZ(&hmap, i, j);
      Vec3 v_pos = vec3_create(i * grid_scale, height, j * grid_scale);
      Vec3 v_normal = VEC3_Y;
      float tiling_factor = 505.0f;
      Vec2 v_uv = vec2((f32)i / width * tiling_factor, (f32)j / height * tiling_factor);
      Vertex v = { .static_3d = { .position = v_pos, .normal = v_normal, .tex_coords = v_uv } };
      Vertex_darray_push(vertices, v);
      index++;
    }
  }
  BufferHandle vertices_handle = GPU_BufferCreate(num_vertices * sizeof(Vertex), BUFFER_VERTEX,
                                                  BUFFER_FLAG_GPU, vertices->data);
  storage->vertex_buffer = vertices_handle;

  u32 quad_count = (width - 1) * (height - 1);
  u32 indices_count = quad_count * 6;
  storage->indices_count = indices_count;
  u32_darray* indices = u32_darray_new(indices_count);
  for (u32 i = 0; i < (width - 1); i++) {     // row
    for (u32 j = 0; j < (height - 1); j++) {  // col
      u32 bot_left = i * width + j;
      u32 top_left = (i + 1) * width + j;
      u32 top_right = (i + 1) * width + (j + 1);
      u32 bot_right = i * width + j + 1;

      // top left tri
      u32_darray_push(indices, top_right);
      u32_darray_push(indices, top_left);
      u32_darray_push(indices, bot_left);

      // bottom right tri
      u32_darray_push(indices, bot_right);
      u32_darray_push(indices, top_right);
      u32_darray_push(indices, bot_left);
    }
  }

  BufferHandle indices_handle =
      GPU_BufferCreate(indices_count * sizeof(u32), BUFFER_INDEX, BUFFER_FLAG_GPU, indices->data);
  storage->index_buffer = indices_handle;
}

Heightmap Heightmap_FromImage(Str8 filepath) {
  size_t max_size = MB(16);
  arena arena = arena_create(malloc(max_size), max_size);
  // str8_opt maybe_file = str8_from_file(&arena, filepath);
  // assert(maybe_file.has_value);

  TextureData hmap_tex = TextureDataLoad(Str8_to_cstr(&arena, filepath), false);

  // arena_free_storage(&arena);

  return (Heightmap){
    .pixel_dimensions = hmap_tex.description.extents,
    .filepath = filepath,
    .image_data = hmap_tex.image_data,
    .num_channels = hmap_tex.description.num_channels,
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

  TerrainUniforms uniforms = { .tex_slot_1 = storage->texture };
  ShaderData terrain_data = { .data = &uniforms, .get_layout = &TerrainUniforms_GetLayout };
  GPU_EncodeBindShaderData(enc, 1, terrain_data);

  GPU_EncodeSetVertexBuffer(enc, storage->vertex_buffer);
  GPU_EncodeSetIndexBuffer(enc, storage->index_buffer);

  GPU_EncodeDrawIndexed(enc, storage->indices_count);
  // glDrawArrays(GL_POINTS, 0, storage->num_vertices);
}

f32 Heightmap_HeightXZ(const Heightmap* hmap, u32 x, u32 z) {
  // its single channel so only one byte per pixel
  size_t position = x * hmap->pixel_dimensions.x + z;
  u8* bytes = hmap->image_data;
  u8 channel = bytes[position];
  float value = (float)channel / 2.0;  /// 255.0;
  return value;
}

ShaderDataLayout TerrainUniforms_GetLayout(void* data) {
  TerrainUniforms* d = data;
  bool has_data = data != NULL;

  ShaderBinding b1 = {
    .label = "TextureSlot1",
    .kind = BINDING_TEXTURE,
    .vis = VISIBILITY_FRAGMENT,
  };

  if (has_data) {
    b1.data.texture.handle = d->tex_slot_1;
  }
  return (ShaderDataLayout){ .bindings = { b1 }, .binding_count = 1 };
}