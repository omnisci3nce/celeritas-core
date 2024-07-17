#include "skybox.h"
#include <assert.h>
#include "glad/glad.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render.h"
#include "render_types.h"

Skybox Skybox_Create(const char** face_paths, int n) {
  assert(n == 6);  // ! we're only supporting a full cubemap for now

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

  VertexDescription pos_only = { .debug_label = "Position only verts" };
  VertexDesc_AddAttr(&pos_only, "inPos", ATTR_F32x3);
  pos_only.use_full_vertex_size = true;

  ShaderData shader_data = { .data = NULL, .get_layout = &Skybox_GetLayout };

  GraphicsPipelineDesc pipeline_desc = {
    .debug_name = "Skybox pipeline",
    .vertex_desc = pos_only,
    .data_layouts = { shader_data },
    .data_layouts_count = 1,
    .vs = {

    },
    .fs = {

    },
    .wireframe = false,
    .depth_test = true,
  };

  return (Skybox){
    .texture = handle,

  };
}