#include <stdlib.h>
#define CEL_PLATFORM_LINUX

#include "defines.h"
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

#endif