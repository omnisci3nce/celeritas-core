#include "grid.h"
#include "file.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "primitives.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render.h"
#include "render_types.h"
#include "render_scene.h"
#include "shader_layouts.h"

void Grid_Init(Grid_Storage* storage) {
  INFO("Infinite Grid initialisation");
  Geometry plane_geo = Geo_CreatePlane(f32x2(1.0, 1.0));
  Mesh plane_mesh = Mesh_Create(&plane_geo, true);
  storage->plane_vertices = plane_mesh.vertex_buffer;

  u32 indices[6] = { 5,4,3,2,1,0};
  storage->plane_indices = GPU_BufferCreate(6 * sizeof(u32),BUFFER_INDEX, BUFFER_FLAG_GPU, &indices);

  GPU_RenderpassDesc rpass_desc = {
    .default_framebuffer = true,
  };
  storage->renderpass = GPU_Renderpass_Create(rpass_desc);

  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  Str8 vert_path = str8("assets/shaders/grid.vert");
  Str8 frag_path = str8("assets/shaders/grid.frag");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  ShaderData camera_data = { .get_layout = &Binding_Camera_GetLayout };

  GraphicsPipelineDesc pipeline_desc = {
    .debug_name = "Infinite grid pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = { camera_data },
    .data_layouts_count = 1,
    .vs = {
      .debug_name = "Grid vertex shader",
      .filepath = vert_path,
      .code = vertex_shader.contents,
    },
    .fs = {
      .debug_name = "Grid fragment shader",
      .filepath = frag_path,
      .code = fragment_shader.contents,
    },
    .wireframe = false,
  };
  storage->pipeline = GPU_GraphicsPipeline_Create(pipeline_desc, storage->renderpass);
}

void Grid_Draw() {
  Grid_Storage* grid = Render_GetGridStorage();
  Grid_Execute(grid);
}

void Grid_Execute(Grid_Storage *storage) {
  WARN("Draw Grid");
  RenderScene* scene = Render_GetScene();
  GPU_CmdEncoder* enc = GPU_GetDefaultEncoder();
  GPU_CmdEncoder_BeginRender(enc, storage->renderpass);
  GPU_EncodeBindPipeline(enc, storage->pipeline);
  Mat4 view, proj;
  u32x2 dimensions = GPU_Swapchain_GetDimensions();
  Camera camera = scene->camera;
  Camera_ViewProj(&camera, (f32)dimensions.x, (f32)dimensions.y, &view, &proj);
  Binding_Camera camera_data = { .view = view,
                                 .projection = proj,
                                 .viewPos = vec4(camera.position.x, camera.position.y,
                                                 camera.position.z, 1.0) };
  GPU_EncodeBindShaderData(
      enc, 0, (ShaderData){ .data = &camera_data, .get_layout = &Binding_Camera_GetLayout });
  GPU_EncodeSetVertexBuffer(enc, storage->plane_vertices);
  GPU_EncodeSetIndexBuffer(enc, storage->plane_indices);
  GPU_EncodeDrawIndexed(enc, 6);
  GPU_CmdEncoder_EndRender(enc);
}