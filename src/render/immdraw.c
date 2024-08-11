#include "immdraw.h"
#include "core.h"
#include "file.h"
#include "log.h"
#include "maths.h"
#include "primitives.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render.h"
#include "shader_layouts.h"

// Forward declares
void Immdraw_Primitive(Transform tf, f32 size, Vec4 colour, bool wireframe, Mesh mesh);

void Immdraw_Init(Immdraw_Storage* storage) {
  INFO("Immediate drawing initialisation");

  // Meshes
  Geometry sphere_geo = Geo_CreateUVsphere(1.0, 16, 16);
  storage->sphere = Mesh_Create(&sphere_geo, false);

  Geometry cube_geo = Geo_CreateCuboid(f32x3(1.0, 1.0, 1.0));
  storage->cube = Mesh_Create(&cube_geo, false);

  Geometry plane_geo = Geo_CreatePlane(f32x2(1.0, 1.0));
  storage->plane = Mesh_Create(&plane_geo, false);

  // Pipeline / material
  VertexDescription vertex_desc = {
    .debug_label = "Immdraw Vertex",
    .use_full_vertex_size = true,
  };
  VertexDesc_AddAttr(&vertex_desc, "position", ATTR_F32x3);
  VertexDesc_AddAttr(&vertex_desc, "normal", ATTR_F32x3);

  const char* vert_path = "assets/shaders/immdraw.vert";
  const char* frag_path = "assets/shaders/immdraw.frag";
  const char* vert_shader = string_from_file(vert_path);
  const char* frag_shader = string_from_file(frag_path);

  ShaderDataLayout camera_data = Binding_Camera_GetLayout(NULL);
  ShaderDataLayout imm_uniform_data = ImmediateUniforms_GetLayout(NULL);

  GraphicsPipelineDesc pipeline_desc = {
    .debug_name = "Immediate Draw Pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = { camera_data, imm_uniform_data },
    .data_layouts_count = 2,
    .vs = { .debug_name = "Immdraw Vertex Shader", .filepath = vert_path, .code = vert_shader },
    .fs = { .debug_name = "Immdraw Fragment Shader", .filepath = frag_path, .code = frag_shader },
    .depth_test = true,
    .wireframe = false,
  };
  GPU_Renderpass* rpass =
      GPU_Renderpass_Create((GPU_RenderpassDesc){ .default_framebuffer = true });
  storage->colour_pipeline = GPU_GraphicsPipeline_Create(pipeline_desc, rpass);
}

void Immdraw_Shutdown(Immdraw_Storage* storage) {
  GraphicsPipeline_Destroy(storage->colour_pipeline);
}

void Immdraw_Sphere(Transform tf, Vec4 colour, bool wireframe) {
  TRACE("Draw sphere");
  Immdraw_Storage* imm = Render_GetImmdrawStorage();
  Immdraw_Primitive(tf, 1.0, colour, wireframe, imm->sphere);
}
void Immdraw_Cuboid(Transform tf, Vec4 colour, bool wireframe) {
  TRACE("Draw cube");
  Immdraw_Storage* imm = Render_GetImmdrawStorage();
  Immdraw_Primitive(tf, 1.0, colour, wireframe, imm->cube);
}
void Immdraw_Plane(Transform tf, Vec4 colour, bool wireframe) {
  TRACE("Draw plane");
  Immdraw_Storage* imm = Render_GetImmdrawStorage();
  Immdraw_Primitive(tf, 1.0, colour, wireframe, imm->plane);
}

void Immdraw_Primitive(Transform tf, f32 size, Vec4 colour, bool wireframe, Mesh mesh) {
  Immdraw_Storage* imm = Render_GetImmdrawStorage();
  GPU_CmdEncoder* enc = GPU_GetDefaultEncoder();

  // begin renderpass
  GPU_CmdEncoder_BeginRender(enc, imm->colour_pipeline->renderpass);
  // bind pipeline
  GPU_EncodeBindPipeline(enc, imm->colour_pipeline);

  // TODO: implement wireframe in other apis
#if defined(CEL_REND_BACKEND_OPENGL)
#include <glad/glad.h>
  if (wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
#endif

  // update uniforms
  ImmediateUniforms uniforms = {
    .model = transform_to_mat(&tf),
    .colour = colour,
  };
  Mat4 view, proj;
  u32x2 dimensions = GPU_Swapchain_GetDimensions();
  RenderScene* scene = Render_GetScene();
  Camera_ViewProj(&scene->camera, (f32)dimensions.x, (f32)dimensions.y, &view, &proj);
  Binding_Camera camera_data = { .view = view,
                                 .projection = proj,
                                 .viewPos = vec4(scene->camera.position.x, scene->camera.position.y,
                                                 scene->camera.position.z, 1.0) };
  GPU_EncodeBindShaderData(enc, 0, Binding_Camera_GetLayout(&camera_data));
  GPU_EncodeBindShaderData(enc, 1, ImmediateUniforms_GetLayout(&uniforms));

  // draw call
  GPU_EncodeSetVertexBuffer(enc, mesh.vertex_buffer);
  GPU_EncodeSetIndexBuffer(enc, mesh.index_buffer);
  GPU_EncodeDrawIndexed(enc, mesh.geometry.index_count);

  // end renderpass
  GPU_CmdEncoder_EndRender(enc);
}
