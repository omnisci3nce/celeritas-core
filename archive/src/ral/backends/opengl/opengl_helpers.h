#pragma once
#include "defines.h"
#include "ral_common.h"
#include "ral_impl.h"
#if defined(CEL_REND_BACKEND_OPENGL)
#include "backend_opengl.h"
#include "file.h"
#include "log.h"
#include "ral_types.h"

#include <glad/glad.h>
#include <glfw3.h>
#include "ral_types.h"

typedef struct opengl_vertex_attr {
  u32 count;
  GLenum data_type;
} opengl_vertex_attr;

static opengl_vertex_attr format_from_vertex_attr(VertexAttribType attr) {
  switch (attr) {
    case ATTR_F32:
      return (opengl_vertex_attr){ .count = 1, .data_type = GL_FLOAT };
    case ATTR_U32:
      return (opengl_vertex_attr){ .count = 1, .data_type = GL_UNSIGNED_INT };
    case ATTR_I32:
      return (opengl_vertex_attr){ .count = 1, .data_type = GL_INT };
    case ATTR_F32x2:
      return (opengl_vertex_attr){ .count = 2, .data_type = GL_FLOAT };
    case ATTR_U32x2:
      // return VK_FORMAT_R32G32_UINT;
    case ATTR_I32x2:
      // return VK_FORMAT_R32G32_UINT;
    case ATTR_F32x3:
      return (opengl_vertex_attr){ .count = 3, .data_type = GL_FLOAT };
    case ATTR_U32x3:
      // return VK_FORMAT_R32G32B32_UINT;
    case ATTR_I32x3:
      // return VK_FORMAT_R32G32B32_SINT;
    case ATTR_F32x4:
      return (opengl_vertex_attr){ .count = 4, .data_type = GL_FLOAT };
    case ATTR_U32x4:
      // return VK_FORMAT_R32G32B32A32_UINT;
    case ATTR_I32x4:
      return (opengl_vertex_attr){ .count = 4, .data_type = GL_INT };
  }
}

static u32 opengl_bindcreate_vao(GPU_Buffer* buf, VertexDescription desc) {
  DEBUG("Vertex format name %s", desc.debug_label);
  // 1. Bind the buffer
  glBindBuffer(GL_ARRAY_BUFFER, buf->id.vbo);
  // 2. Create new VAO
  u32 vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Attributes
  u32 attr_count = desc.attributes_count;
  // printf("N attributes %d\n", attr_count);
  u64 offset = 0;
  size_t vertex_size = desc.use_full_vertex_size ? sizeof(Vertex) : VertexDesc_CalcStride(&desc);
  for (u32 i = 0; i < desc.attributes_count; i++) {
    opengl_vertex_attr format = format_from_vertex_attr(desc.attributes[i]);
    glVertexAttribPointer(i, format.count, format.data_type, GL_FALSE, vertex_size, (void*)offset);
    TRACE(" %d %d %d %d %d %s", i, format.count, format.data_type, vertex_size, offset,
          desc.attr_names[i]);
    glEnableVertexAttribArray(i);  // nth index
    size_t this_offset = VertexAttribSize(desc.attributes[i]);
    // printf("offset total %lld this attr %zu\n", offset, this_offset);
    offset += this_offset;
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return vao;
}

static u32 shader_create_separate(const char* vert_shader, const char* frag_shader) {
  INFO("Load shaders at %s and %s", vert_shader, frag_shader);
  int success;
  char info_log[512];

  u32 vertex = glCreateShader(GL_VERTEX_SHADER);
  const char* vertex_shader_src = string_from_file(vert_shader);
  if (vertex_shader_src == NULL) {
    ERROR("EXIT: couldnt load shader");
    exit(-1);
  }
  glShaderSource(vertex, 1, &vertex_shader_src, NULL);
  glCompileShader(vertex);
  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, info_log);
    printf("%s\n", info_log);
    ERROR("EXIT: vertex shader compilation failed");
    exit(-1);
  }

  // fragment shader
  u32 fragment = glCreateShader(GL_FRAGMENT_SHADER);
  const char* fragment_shader_src = string_from_file(frag_shader);
  if (fragment_shader_src == NULL) {
    ERROR("EXIT: couldnt load shader");
    exit(-1);
  }
  glShaderSource(fragment, 1, &fragment_shader_src, NULL);
  glCompileShader(fragment);
  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, NULL, info_log);
    printf("%s\n", info_log);
    ERROR("EXIT: fragment shader compilation failed");
    exit(-1);
  }

  u32 shader_prog;
  shader_prog = glCreateProgram();

  glAttachShader(shader_prog, vertex);
  glAttachShader(shader_prog, fragment);
  glLinkProgram(shader_prog);
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  free((char*)vertex_shader_src);
  free((char*)fragment_shader_src);

  return shader_prog;
}

static GLenum opengl_tex_type(GPU_TextureType tex_type) {
  switch (tex_type) {
    case TEXTURE_TYPE_2D:
      return GL_TEXTURE_2D;
    case TEXTURE_TYPE_CUBE_MAP:
      return GL_TEXTURE_CUBE_MAP;
    default:
      return GL_TEXTURE_2D;
  }
}

static GLenum opengl_prim_topology(PrimitiveTopology t) {
  switch (t) {
    case CEL_POINT:
      return GL_POINT;
    case CEL_LINE:
      return GL_LINES;
    case CEL_LINE_STRIP:
      return GL_LINE_STRIP;
    case CEL_TRI:
      return GL_TRIANGLES;
    case CEL_TRI_STRIP:
      return GL_TRIANGLE_STRIP;
    case PRIMITIVE_TOPOLOGY_COUNT:
      WARN("Invalid PrimitiveTopology value");
      break;
  }
}

#endif
