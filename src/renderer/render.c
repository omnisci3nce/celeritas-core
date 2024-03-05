#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "render.h"

#include <glad/glad.h>
#include <glfw3.h>

#include "log.h"
#include "render_backend.h"

material DEFAULT_MATERIAL = { 0 };

bool renderer_init(renderer* ren) {
  INFO("Renderer init");

  // NOTE: all platforms use GLFW at the moment but thats subject to change
  glfwInit();

  // glfw window creation
  GLFWwindow* window = glfwCreateWindow(ren->config.scr_width, ren->config.scr_height,
                                        ren->config.window_name, NULL, NULL);
  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return false;
  }
  ren->window = window;

  glfwMakeContextCurrent(ren->window);

  if (!gfx_backend_init(ren)) {
    FATAL("Couldnt load graphics api backend");
    return false;
  }

  return true;
}

void render_frame_begin(renderer* ren) {
  vec3 color = ren->config.clear_colour;
  clear_screen(color);
}
void render_frame_end(renderer* ren) {
  // present frame
  glfwSwapBuffers(ren->window);
  glfwPollEvents();
}

void default_material_init() {
  INFO("Load default material")
  DEFAULT_MATERIAL.ambient_colour = (vec3){ 0.5, 0.5, 0.5 };
  DEFAULT_MATERIAL.diffuse = (vec3){ 0.8, 0.8, 0.8 };
  DEFAULT_MATERIAL.specular = (vec3){ 1.0, 1.0, 1.0 };
  DEFAULT_MATERIAL.diffuse_texture = texture_data_load("assets/textures/white1x1.png", false);
  DEFAULT_MATERIAL.specular_texture = texture_data_load("assets/textures/black1x1.png", false);
  DEFAULT_MATERIAL.spec_exponent = 32.0;
  strcpy(DEFAULT_MATERIAL.name, "Default");
  texture_data_upload(&DEFAULT_MATERIAL.diffuse_texture);
  texture_data_upload(&DEFAULT_MATERIAL.specular_texture);
}

texture texture_data_load(const char* path, bool invert_y) {
  TRACE("Load texture %s", path);

  // load the file data
  // texture loading
  int width, height, num_channels;
  stbi_set_flip_vertically_on_load(invert_y);

#pragma GCC diagnostic ignored "-Wpointer-sign"
  char* data = stbi_load(path, &width, &height, &num_channels, 0);
  if (data) {
    DEBUG("loaded texture: %s", path);
  } else {
    WARN("failed to load texture");
  }

  unsigned int channel_type;
  if (num_channels == 4) {
    channel_type = GL_RGBA;
  } else {
    channel_type = GL_RGB;
  }

  return (texture){ .texture_id = 0,
                    .width = width,
                    .height = height,
                    .channel_count = num_channels,
                    .channel_type = channel_type,
                    .name = "TODO: Texture names",
                    .image_data = data };
}

void texture_data_upload(texture* tex) {
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
