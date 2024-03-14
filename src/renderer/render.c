#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "render.h"
#include "render_types.h"

#include <glad/glad.h>
#include <glfw3.h>

#include "defines.h"
#include "log.h"
#include "maths.h"
#include "render_backend.h"

// FIXME: get rid of these and store dynamic screen realestate
//        in renderer
#define SCR_WIDTH 1080
#define SCR_HEIGHT 800

material DEFAULT_MATERIAL = { 0 };

bool renderer_init(renderer* ren) {
  INFO("Renderer init");

  // NOTE: all platforms use GLFW at the moment but thats subject to change
  glfwInit();

  DEBUG("init graphics api (OpenGL) backend");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

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

  ren->blinn_phong =
      shader_create_separate("assets/shaders/blinn_phong.vert", "assets/shaders/blinn_phong.frag");

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

void draw_model(renderer* ren, camera* camera, model* model, transform tf, scene* scene) {
  // TRACE("Drawing model: %s", model->name);
  mat4 view;
  mat4 proj;
  camera_view_projection(camera, SCR_HEIGHT, SCR_WIDTH, &view, &proj);

  set_shader(ren->blinn_phong);

  // set camera uniform
  uniform_vec3f(ren->blinn_phong.program_id, "viewPos", &camera->position);
  // set light uniforms
  dir_light_upload_uniforms(ren->blinn_phong, &scene->dir_light);
  for (int i = 0; i < scene->n_point_lights; i ++) {
    point_light_upload_uniforms(ren->blinn_phong, &scene->point_lights[i], '0' + i);
  }

  for (size_t i = 0; i < mesh_darray_len(model->meshes); i++) {
    mesh* m = &model->meshes->data[i];
    if (vertex_darray_len(m->vertices) == 0) {
      continue;
    }
    // TRACE("Drawing mesh %d", i);
    material* mat = &model->materials->data[m->material_index];
    draw_mesh(ren, m, tf, mat, &view, &proj);
  }
}

void draw_mesh(renderer* ren, mesh* mesh, transform tf, material* mat, mat4* view, mat4* proj) {
  shader lighting_shader = ren->blinn_phong;

  // bind buffer
  bind_mesh_vertex_buffer(ren->backend_state, mesh);

  // bind textures
  bind_texture(lighting_shader, &mat->diffuse_texture, 0);   // bind to slot 0
  bind_texture(lighting_shader, &mat->specular_texture, 1);  // bind to slot 1
  uniform_f32(lighting_shader.program_id, "material.shininess", 32.);

  // upload model transform
  mat4 trans = mat4_translation(tf.position);
  mat4 rot = mat4_rotation(tf.rotation);
  mat4 scale = mat4_scale(tf.scale);
  mat4 model_tf = mat4_mult(trans, mat4_mult(rot, scale));

  uniform_mat4f(lighting_shader.program_id, "model", &model_tf);
  // upload view & projection matrices
  uniform_mat4f(lighting_shader.program_id, "view", view);
  uniform_mat4f(lighting_shader.program_id, "projection", proj);

  // draw triangles
  u32 num_vertices = vertex_darray_len(mesh->vertices);
  draw_primitives(CEL_PRIMITIVE_TOPOLOGY_TRIANGLE, 0, num_vertices);
}

void model_upload_meshes(renderer* ren, model* model) {
  INFO("Upload mesh vertex data to GPU for model %s", model->name);

  size_t num_meshes = mesh_darray_len(model->meshes);
  u32 VBOs[num_meshes];
  u32 VAOs[num_meshes];
  glGenBuffers(num_meshes, VBOs);
  glGenVertexArrays(num_meshes, VAOs);

  u64 total_verts = 0;

  TRACE("num meshes %d", num_meshes);

  // upload each mesh to the GPU
  for (int mesh_i = 0; mesh_i < num_meshes; mesh_i++) {
    model->meshes->data[mesh_i].vao = VAOs[mesh_i];
    model->meshes->data[mesh_i].vbo = VBOs[mesh_i];
    // 3. bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[mesh_i]);

    size_t num_vertices = vertex_darray_len(model->meshes->data[mesh_i].vertices);
    // TRACE("Uploading vertex array data: %d verts", num_vertices);
    total_verts += num_vertices;

    // TODO: convert this garbage into a function
    f32 verts[num_vertices * 8];
    // for each face
    for (int i = 0; i < (num_vertices / 3); i++) {
      // for each vert in face
      for (int j = 0; j < 3; j++) {
        size_t stride = (i * 24) + j * 8;
        // printf("i: %d, stride: %ld, loc %d\n", i, stride, i * 3 + j);
        vertex vert = model->meshes->data[mesh_i].vertices->data[i];
        // printf("pos %f %f %f\n", vert.position.x, vert.position.y, vert.position.z);
        // printf("norm %f %f %f\n", vert.normal.x, vert.normal.y, vert.normal.z);
        // printf("tex %f %f\n", vert.uv.x, vert.uv.y);
        verts[stride + 0] =
            ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].position.x;
        verts[stride + 1] =
            ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].position.y;
        verts[stride + 2] =
            ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].position.z;
        verts[stride + 3] =
            ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].normal.x;
        verts[stride + 4] =
            ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].normal.y;
        verts[stride + 5] =
            ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].normal.z;
        verts[stride + 6] = ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].uv.x;
        verts[stride + 7] = ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].uv.y;
      }
    }

    // 4. upload data
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // 5. cont. set mesh vertex layout
    glBindVertexArray(model->meshes->data[mesh_i].vao);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal vector attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // tex coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
  }

  INFO("Uploaded %d submeshes with a total of %d vertices\n", num_meshes, total_verts);

  // 6. reset buffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);
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

void dir_light_upload_uniforms(shader shader, directional_light *light) {
  uniform_vec3f(shader.program_id, "dirLight.direction", &light->direction);
  uniform_vec3f(shader.program_id, "dirLight.ambient", &light->ambient);
  uniform_vec3f(shader.program_id, "dirLight.diffuse", &light->diffuse);
  uniform_vec3f(shader.program_id, "dirLight.specular", &light->specular);
}

void point_light_upload_uniforms(shader shader, point_light *light, char index) {
  char position_str[] = "pointLights[x].position";
  position_str[12] = (char)index;
  char ambient_str[] = "pointLights[x].ambient";
  ambient_str[12] = (char)index;
  char diffuse_str[] = "pointLights[x].diffuse";
  diffuse_str[12] = (char)index;
  char specular_str[] = "pointLights[x].specular";
  specular_str[12] = (char)index;
  char constant_str[] = "pointLights[x].constant";
  constant_str[12] = (char)index;
  char linear_str[] = "pointLights[x].linear";
  linear_str[12] = (char)index;
  char quadratic_str[] = "pointLights[x].quadratic";
  quadratic_str[12] = (char)index;
  uniform_vec3f(shader.program_id, position_str, &light->position);
  uniform_vec3f(shader.program_id, ambient_str, &light->ambient);
  uniform_vec3f(shader.program_id, diffuse_str, &light->diffuse);
  uniform_vec3f(shader.program_id, specular_str, &light->specular);
  uniform_f32(shader.program_id, constant_str, light->constant);
  uniform_f32(shader.program_id, linear_str, light->linear);
  uniform_f32(shader.program_id, quadratic_str, light->quadratic);
}