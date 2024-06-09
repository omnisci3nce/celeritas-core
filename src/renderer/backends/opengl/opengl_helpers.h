#if defined(CEL_REND_BACKEND_OPENGL)
#pragma once
#include "backend_opengl.h"
#include "log.h"
#include "ral.h"
#include "ral_types.h"

#include <glad/glad.h>
#include <glfw3.h>
#include "ral_types.h"
typedef struct opengl_vertex_attr {
  u32 count;
  GLenum data_type;
} opengl_vertex_attr;

static opengl_vertex_attr format_from_vertex_attr(vertex_attrib_type attr) {
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

static u32 opengl_bindcreate_vao(gpu_buffer* buf, vertex_description desc) {
  // 1. Bind the buffer
  glBindBuffer(GL_ARRAY_BUFFER, buf->id.vbo);
  // 2. Create new VAO
  u32 vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Attributes
  u32 attr_count = desc.attributes_count;
  printf("N attributes %d\n", attr_count);
  u64 offset = 0;
  size_t vertex_size = desc.stride;
  for (u32 i = 0; i < desc.attributes_count; i++) {
    opengl_vertex_attr format = format_from_vertex_attr(desc.attributes[i]);
    glVertexAttribPointer(i, format.count, format.data_type, GL_FALSE, vertex_size, (void*)offset);
    TRACE(" %d %d %d %d %d %s", i, format.count, format.data_type, vertex_size, offset,
          desc.attr_names[i]);
    glEnableVertexAttribArray(i);  // nth index
    size_t this_offset = vertex_attrib_size(desc.attributes[i]);
    printf("offset total %lld this attr %ld\n", offset, this_offset);
    offset += this_offset;
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return vao;
}

#endif
