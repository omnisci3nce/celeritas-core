#include <stdlib.h>
#define CEL_PLATFORM_LINUX

#include "defines.h"
#include "file.h"
#include "log.h"
#include "maths_types.h"
#include "render_types.h"

#if CEL_REND_BACKEND_OPENGL

#include <glad/glad.h>

#include <glfw3.h>

/** @brief Internal backend state */
typedef struct opengl_state {
} opengl_state;

bool gfx_backend_init(renderer *ren) {
  INFO("loading OpenGL backend");

  // glfwInit(); // Already handled in `renderer_init`
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    ERROR("Failed to initialise GLAD \n");

    return false;
  }

  glEnable(GL_DEPTH_TEST);

  opengl_state *internal = malloc(sizeof(opengl_state));
  ren->backend_state = (void *)internal;

  return true;
}
void gfx_backend_shutdown(renderer *ren) {}

void uniform_vec3f(u32 program_id, const char *uniform_name, vec3 *value) {
  glUniform3fv(glGetUniformLocation(program_id, uniform_name), 1, &value->x);
}
void uniform_f32(u32 program_id, const char *uniform_name, f32 value) {
  glUniform1f(glGetUniformLocation(program_id, uniform_name), value);
}
void uniform_i32(u32 program_id, const char *uniform_name, i32 value) {
  glUniform1i(glGetUniformLocation(program_id, uniform_name), value);
}
void uniform_mat4f(u32 program_id, const char *uniform_name, mat4 *value) {
  glUniformMatrix4fv(glGetUniformLocation(program_id, uniform_name), 1, GL_FALSE, value->data);
}

void clear_screen(vec3 colour) {
  glClearColor(colour.x, colour.y, colour.z, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void texture_data_upload(texture *tex) {
  printf("Texture name %s\n", tex->name);
  TRACE("Upload texture data");
  u32 texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  tex->texture_id = texture_id;

  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_REPEAT);  // set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->width, tex->height, 0, tex->channel_type,
               GL_UNSIGNED_BYTE, tex->image_data);
  glGenerateMipmap(GL_TEXTURE_2D);
  DEBUG("Freeing texture image data after uploading to GPU");
  // stbi_image_free(tex->image_data);  // data is on gpu now so we dont need it around
}

void bind_texture(shader s, texture *tex, u32 slot) {
  // printf("bind texture slot %d with texture id %d \n", slot, tex->texture_id);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, tex->texture_id);
}

void bind_mesh_vertex_buffer(void *_backend, mesh *mesh) { glBindVertexArray(mesh->vao); }

static inline GLenum to_gl_prim_topology(enum cel_primitive_topology primitive) {
  switch (primitive) {
    case CEL_PRIMITIVE_TOPOLOGY_TRIANGLE:
      return GL_TRIANGLES;
    case CEL_PRIMITIVE_TOPOLOGY_POINT:
    case CEL_PRIMITIVE_TOPOLOGY_LINE:
    case CEL_PRIMITIVE_TOPOLOGY_LINE_STRIP:
    case CEL_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
    case CEL_PRIMITIVE_TOPOLOGY_COUNT:
      break;
  }
}

void draw_primitives(cel_primitive_topology primitive, u32 start_index, u32 count) {
  u32 gl_primitive = to_gl_prim_topology(primitive);
  glDrawArrays(gl_primitive, start_index, count);
}

shader shader_create_separate(const char *vert_shader, const char *frag_shader) {
  INFO("Load shaders at %s and %s", vert_shader, frag_shader);
  int success;
  char info_log[512];

  u32 vertex = glCreateShader(GL_VERTEX_SHADER);
  const char *vertex_shader_src = string_from_file(vert_shader);
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
  const char *fragment_shader_src = string_from_file(frag_shader);
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
  free((char *)vertex_shader_src);
  free((char *)fragment_shader_src);

  shader s = { .program_id = shader_prog };
  return s;
}

void set_shader(shader s) { glUseProgram(s.program_id); }

#endif