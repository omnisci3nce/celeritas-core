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

float skyboxVertices[] = {
  // positions
  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
  1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f
};

static const char* faces[6] = { "assets/demo/skybox/right.jpg", "assets/demo/skybox/left.jpg",
                                "assets/demo/skybox/top.jpg",   "assets/demo/skybox/bottom.jpg",
                                "assets/demo/skybox/front.jpg", "assets/demo/skybox/back.jpg" };

Skybox Skybox_Create(const char** face_paths, int n) {
  INFO("Creating a skybox");
  CASSERT_MSG(
      n == 6,
      "We only support full cubemaps for now");  // ! we're only supporting a full cubemap for now

  // -- cube verts
  Geometry geom = { .format = VERTEX_POS_ONLY,  // doesnt matter
                    .has_indices = false,
                    .indices = NULL,
                    .vertices = Vertex_darray_new(36) };
  for (u32 i = 0; i < (36 * 3); i += 3) {
    Vertex_darray_push(
        geom.vertices,
        (Vertex){ .pos_only = { .position = vec3(skyboxVertices[i], skyboxVertices[i + 1],
                                                 skyboxVertices[i + 2]) } });
  }
  Mesh cube = Mesh_Create(&geom, false);

  // -- cubemap texture
  TextureHandle handle;
  GPU_Texture* tex = GPU_TextureAlloc(&handle);
  glBindTexture(GL_TEXTURE_CUBE_MAP, tex->id);

  for (unsigned int i = 0; i < n; i++) {
    TextureData data = TextureDataLoad(face_paths[i], false);
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

  ShaderDataLayout camera_data = Binding_Camera_GetLayout(NULL);
  ShaderDataLayout shader_data = Skybox_GetLayout(NULL);

  VertexDescription builder = { .debug_label = "pos only" };
  VertexDesc_AddAttr(&builder, "inPosition", ATTR_F32x3);
  builder.use_full_vertex_size = true;

  GraphicsPipelineDesc pipeline_desc = {
    .debug_name = "Skybox pipeline",
    .vertex_desc = builder,
    .data_layouts = { shader_data, camera_data },
    .data_layouts_count = 2,
    .vs = { .debug_name = "Skybox Vertex Shader",
            .filepath = vert_path,
            .code = vertex_shader.contents },
    .fs = { .debug_name = "Skybox Fragment Shader",
            .filepath = frag_path,
            .code = fragment_shader.contents },
    .wireframe = false,
    .depth_test = true,
  };

  GPU_Pipeline* pipeline = GPU_GraphicsPipeline_Create(pipeline_desc, pass);

  return (Skybox){ .cube = cube, .texture = handle, .pipeline = pipeline };
}

Skybox Skybox_Default() { return Skybox_Create(faces, 6); }

void Skybox_Draw(Skybox* skybox, Camera camera) {
  GPU_CmdEncoder* enc = GPU_GetDefaultEncoder();
  glDepthFunc(GL_LEQUAL);
  GPU_CmdEncoder_BeginRender(enc, skybox->pipeline->renderpass);
  GPU_EncodeBindPipeline(enc, skybox->pipeline);
  GPU_EncodeSetDefaults(enc);

  // Shader data

  Mat4 view, proj;
  u32x2 dimensions = GPU_Swapchain_GetDimensions();
  Camera_ViewProj(&camera, dimensions.x, dimensions.y, &view, &proj);
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
  GPU_EncodeBindShaderData(enc, 0, Binding_Camera_GetLayout(&camera_data));

  SkyboxUniforms uniforms = { .cubemap = skybox->texture };
  ShaderDataLayout skybox_data = Skybox_GetLayout(&uniforms);
  GPU_EncodeBindShaderData(enc, 0, skybox_data);

  GPU_EncodeSetVertexBuffer(enc, skybox->cube.vertex_buffer);
  GPU_EncodeSetIndexBuffer(enc, skybox->cube.index_buffer);

  GPU_EncodeDraw(enc, 36);

  GPU_CmdEncoder_EndRender(enc);
  glDepthFunc(GL_LESS);
}
