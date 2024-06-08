#pragma once
#include <glad/glad.h>
#include <glfw3.h>
#include "ral_types.h"
typedef struct opengl_vertex_attr {
  u32 count;
  GLenum data_type;
} opengl_vertex_attr;
opengl_vertex_attr format_from_vertex_attr(vertex_attrib_type attr) {
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