#include "skybox.h"
#include <assert.h>
#include "file.h"
#include "glad/glad.h"
#include "log.h"
#include "maths.h"
#include "primitives.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render.h"
#include "render_types.h"
#include "shader_layouts.h"

Skybox Skybox_Create(const char** face_paths, int n) {
  INFO("Creating a skybox");
  assert(n == 6);  // ! we're only supporting a full cubemap for now

  // -- cube verts
  Geometry geom = Geo_CreateCuboid(f32x3(1.0, 1.0, 1.0));
  Mesh cube = Mesh_Create(&geom, false);

  // -- cubemap texture
  TextureHandle handle;
  GPU_Texture* tex = GPU_TextureAlloc(&handle);
  glBindTexture(GL_TEXTURE_CUBE_MAP, tex->id);

  int width, height, nrChannels;
  // unsigned char *data;
  for (unsigned int i = 0; i < n; i++) {
    TextureData data = TextureDataLoad(
        face_paths[i],
        false);  // stbi_load(textures_faces[i].c_str(), &width, &height, &nrChannels, 0);
    assert(data.description.format == TEXTURE_FORMAT_8_8_8_RGB_UNORM);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, data.description.extents.x,
                 data.description.extents.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data.image_data);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  // shader pipeline
  GPU_RenderpassDesc rpass_desc = {
    .default_framebuffer = true,
  };
  GPU_Renderpass* pass = GPU_Renderpass_Create(rpass_desc);

  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  Str8 vert_path = str8("assets/shaders/skybox.vert");
  Str8 frag_path = str8("assets/shaders/skybox.frag");
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  // VertexDescription pos_only = { .debug_label = "Position only verts" };
  // VertexDesc_AddAttr(&pos_only, "inPos", ATTR_F32x3);
  // pos_only.use_full_vertex_size = true;

  ShaderData camera_data = { .data = NULL, .get_layout = &Binding_Camera_GetLayout};
  ShaderData shader_data = { .data = NULL, .get_layout = &Skybox_GetLayout };

  GraphicsPipelineDesc pipeline_desc = {
    .debug_name = "Skybox pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = { camera_data, shader_data },
    .data_layouts_count = 2,
    .vs = {
      .debug_name = "Skybox Vertex Shader",
      .filepath = vert_path,
      .code = vertex_shader.contents
    },
    .fs = {
      .debug_name = "Skybox Fragment Shader",
      .filepath = frag_path,
      .code = fragment_shader.contents
    },
    .wireframe = false,
    .depth_test = true,
  };

  GPU_Pipeline* pipeline = GPU_GraphicsPipeline_Create(pipeline_desc, pass);

  return (Skybox){ .cube = cube, .texture = handle, .pipeline = pipeline };
}

void Skybox_Draw(Skybox* skybox, Camera camera) {
  GPU_CmdEncoder* enc = GPU_GetDefaultEncoder();
  GPU_CmdEncoder_BeginRender(enc, skybox->pipeline->renderpass);
  GPU_EncodeBindPipeline(enc, skybox->pipeline);
  GPU_EncodeSetDefaults(enc);

  // Shader data

  Mat4 view, proj;
  Camera_ViewProj(&camera, 1000, 1000, &view, &proj);
  Mat4 new = mat4_ident();
  new.data[0] = view.data[0];
  new.data[1] = view.data[1];
  new.data[2] = view.data[2];
  new.data[4] = view.data[4];
  new.data[5] = view.data[5];
  new.data[6] = view.data[6];
  new.data[8] = view.data[8];
  new.data[9] = view.data[9];
  new.data[10] = view.data[10];

  Binding_Camera camera_data = { .view = new,
                                 .projection = proj,
                                 .viewPos = vec4(camera.position.x, camera.position.y,
                                                 camera.position.z, 1.0) };
  GPU_EncodeBindShaderData(
      enc, 0, (ShaderData){ .data = &camera_data, .get_layout = &Binding_Camera_GetLayout });


  SkyboxUniforms uniforms = {  .cubemap = skybox->texture };
  ShaderData skybox_data = { .data = &uniforms, .get_layout = &Skybox_GetLayout };
  GPU_EncodeBindShaderData(enc, 0, skybox_data);

  GPU_EncodeSetVertexBuffer(enc, skybox->cube.vertex_buffer);
  GPU_EncodeSetIndexBuffer(enc, skybox->cube.index_buffer);
  glDepthFunc(GL_LEQUAL);
  GPU_EncodeDrawIndexed(enc, skybox->cube.geometry.indices->len);
  glDepthFunc(GL_LESS);

  GPU_CmdEncoder_EndRender(enc);
}
