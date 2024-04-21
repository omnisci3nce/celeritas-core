#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "animation.h"
#include "maths_types.h"
#include "mem.h"
#include "transform_hierarchy.h"
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
#define SCR_WIDTH 1000
#define SCR_HEIGHT 1000

material DEFAULT_MATERIAL = { 0 };

bool renderer_init(renderer* ren) {
  INFO("Renderer init");

  // NOTE: all platforms use GLFW at the moment but thats subject to change
  glfwInit();

#if defined(CEL_REND_BACKEND_OPENGL)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#elif defined(CEL_REND_BACKEND_VULKAN)
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

  // glfw window creation
  GLFWwindow* window = glfwCreateWindow(ren->config.scr_width, ren->config.scr_height,
                                        ren->config.window_name, NULL, NULL);
  if (window == NULL) {
    ERROR("Failed to create GLFW window\n");
    glfwTerminate();
    return false;
  }
  ren->window = window;

  glfwMakeContextCurrent(ren->window);

  DEBUG("init graphics api backend");
  if (!gfx_backend_init(ren)) {
    FATAL("Couldnt load graphics api backend");
    return false;
  }

  ren->blinn_phong =
      shader_create_separate("assets/shaders/blinn_phong.vert", "assets/shaders/blinn_phong.frag");

  ren->skinned =
      shader_create_separate("assets/shaders/skinned.vert", "assets/shaders/blinn_phong.frag");

  default_material_init();

  return true;
}

void renderer_shutdown(renderer* ren) {}

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

void model_destroy(model* model) {
  TRACE("Freeing all data for model %s", model->name);
  arena_free_all(&model->animation_data_arena);
  arena_free_storage(&model->animation_data_arena);
  mesh_darray_free(model->meshes);
  material_darray_free(model->materials);
  if (model->is_uploaded) {
    // Delete gpu buffer data
    for (u32 i = 0; i < mesh_darray_len(model->meshes); i++) {
      // FIXME: dont leak Opengl
      glDeleteBuffers(1, &model->meshes->data[i].vbo);
      glDeleteVertexArrays(1, &model->meshes->data[i].vao);
    }
  }
}

typedef struct draw_ctx {
  model_darray* models;
  renderer* ren;
  camera* cam;
  scene* scene;
} draw_ctx;
bool draw_scene_node(transform_node* node, void* ctx_data) {
  if (!node || !node->parent) return true;
  draw_ctx* ctx = ctx_data;
  model* m = &ctx->models->data[node->model.raw];
  draw_model(ctx->ren, ctx->cam, m, &node->world_matrix_tf, ctx->scene);
  return true;
}

void draw_scene(arena* frame, model_darray* models, renderer* ren, camera* camera,
                transform_hierarchy* tfh, scene* scene) {
  draw_ctx* ctx = arena_alloc(frame, sizeof(draw_ctx));
  ctx->models = models;
  ctx->ren = ren;
  ctx->cam = camera;
  ctx->scene = scene;
  transform_hierarchy_dfs(transform_hierarchy_root_node(tfh), draw_scene_node, true, ctx);
}

void draw_model(renderer* ren, camera* camera, model* model, mat4* model_tf, scene* scene) {
  // TRACE("Drawing model: %s", model->name);
  mat4 view;
  mat4 proj;
  camera_view_projection(camera, SCR_HEIGHT, SCR_WIDTH, &view, &proj);

  set_shader(ren->blinn_phong);

  // set camera uniform
  uniform_vec3f(ren->blinn_phong.program_id, "viewPos", &camera->position);
  // set light uniforms
  dir_light_upload_uniforms(ren->blinn_phong, &scene->dir_light);
  for (int i = 0; i < scene->n_point_lights; i++) {
    point_light_upload_uniforms(ren->blinn_phong, &scene->point_lights[i], '0' + i);
  }

  for (size_t i = 0; i < mesh_darray_len(model->meshes); i++) {
    mesh* m = &model->meshes->data[i];
    if (vertex_darray_len(m->vertices) == 0) {
      continue;
    }
    // TRACE("Drawing mesh %d", i);
    material* mat = &model->materials->data[m->material_index];
    draw_mesh(ren, m, model_tf, mat, &view, &proj);
  }
}

void draw_mesh(renderer* ren, mesh* mesh, mat4* model_tf, material* mat, mat4* view, mat4* proj) {
  shader lighting_shader = ren->blinn_phong;

  // bind buffer
  bind_mesh_vertex_buffer(ren->backend_state, mesh);

  // bind textures
  bind_texture(lighting_shader, &mat->diffuse_texture, 0);   // bind to slot 0
  bind_texture(lighting_shader, &mat->specular_texture, 1);  // bind to slot 1
  uniform_f32(lighting_shader.program_id, "material.shininess", 32.);

  // upload model transform
  // mat4 trans = mat4_translation(tf.position);
  // mat4 rot = mat4_rotation(tf.rotation);
  // mat4 scale = mat4_scale(tf.scale);
  // mat4 model_tf = mat4_mult(trans, mat4_mult(rot, scale));

  uniform_mat4f(lighting_shader.program_id, "model", model_tf);
  // upload view & projection matrices
  uniform_mat4f(lighting_shader.program_id, "view", view);
  uniform_mat4f(lighting_shader.program_id, "projection", proj);

  // draw triangles
  u32 num_vertices = vertex_darray_len(mesh->vertices);
  draw_primitives(CEL_PRIMITIVE_TOPOLOGY_TRIANGLE, 0, num_vertices);
}

void draw_skinned_mesh(renderer* ren, mesh* mesh, transform tf, material* mat, mat4* view,
                       mat4* proj) {
  shader lighting_shader = ren->skinned;

  // bind buffer
  bind_mesh_vertex_buffer(ren->backend_state, mesh);

  // bind textures
  bind_texture(lighting_shader, &mat->diffuse_texture, 0);   // bind to slot 0
  bind_texture(lighting_shader, &mat->specular_texture, 1);  // bind to slot 1

  // Uniforms
  uniform_f32(lighting_shader.program_id, "material.shininess", 32.);
  mat4 trans = mat4_translation(tf.position);
  mat4 rot = mat4_rotation(tf.rotation);
  mat4 scale = mat4_scale(tf.scale);
  mat4 model_tf = mat4_mult(trans, mat4_mult(rot, scale));
  uniform_mat4f(lighting_shader.program_id, "model", &model_tf);
  uniform_mat4f(lighting_shader.program_id, "view", view);
  uniform_mat4f(lighting_shader.program_id, "projection", proj);

  // bone transforms
  size_t n_bones = mesh->bones->len;

  // for now assume correct ordering
  mat4* bone_transforms = malloc(n_bones * sizeof(mat4));
  mat4 parent = mat4_ident();
  for (int bone_i = 0; bone_i < n_bones; bone_i++) {
    joint j = mesh->bones->data[bone_i];
    transform tf = mesh->bones->data[bone_i].transform_components;
    tf.position.y = -tf.position.y;
    mat4 local = transform_to_mat(&tf);
    mat4 inverse = j.inverse_bind_matrix;
    inverse.data[13] = -inverse.data[13];
    mat4 intemediate = mat4_mult(local, inverse);

    bone_transforms[bone_i] = intemediate;
    parent = bone_transforms[bone_i];
  }

  // premultiply the inverses
  // for (int bone_i = 0; bone_i < n_bones; bone_i++) {
  //   joint j = mesh->bones->data[bone_i];
  //   // bone_transforms[bone_i] = mat4_mult(bone_transforms[bone_i], j.inverse_bind_matrix);
  //   bone_transforms[bone_i] = mat4_mult(bone_transforms[bone_i], j.inverse_bind_matrix);
  // }

  glUniformMatrix4fv(glGetUniformLocation(lighting_shader.program_id, "boneMatrices"), n_bones,
                     GL_FALSE, &bone_transforms->data[0]);

  free(bone_transforms);

  // draw triangles
  u32 num_vertices = vertex_darray_len(mesh->vertices);
  draw_primitives(CEL_PRIMITIVE_TOPOLOGY_TRIANGLE, 0, num_vertices);
}

void draw_skinned_model(renderer* ren, camera* cam, model* model, transform tf, scene* scene) {
  mat4 view;
  mat4 proj;
  camera_view_projection(cam, SCR_HEIGHT, SCR_WIDTH, &view, &proj);

  set_shader(ren->skinned);

  // set camera uniform
  uniform_vec3f(ren->skinned.program_id, "viewPos", &cam->position);
  // set light uniforms
  dir_light_upload_uniforms(ren->skinned, &scene->dir_light);
  for (int i = 0; i < scene->n_point_lights; i++) {
    point_light_upload_uniforms(ren->skinned, &scene->point_lights[i], '0' + i);
  }

  for (size_t i = 0; i < mesh_darray_len(model->meshes); i++) {
    mesh* m = &model->meshes->data[i];
    if (vertex_darray_len(m->vertices) == 0) {
      continue;
    }
    // material* mat = &model->materials->data[m->material_index];
    material* mat = &DEFAULT_MATERIAL;
    draw_skinned_mesh(ren, m, tf, mat, &view, &proj);
  }
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
    mesh mesh = model->meshes->data[mesh_i];
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
    // for (int i = 0; i < (num_vertices / 3); i++) {
    //   // for each vert in face
    //   for (int j = 0; j < 3; j++) {
    //     size_t stride = (i * 24) + j * 8;
    //     // printf("i: %d, stride: %ld, loc %d\n", i, stride, i * 3 + j);
    //     vertex vert = model->meshes->data[mesh_i].vertices->data[i];
    //     // printf("pos %f %f %f\n", vert.position.x, vert.position.y, vert.position.z);
    //     // printf("norm %f %f %f\n", vert.normal.x, vert.normal.y, vert.normal.z);
    //     // printf("tex %f %f\n", vert.uv.x, vert.uv.y);
    //     verts[stride + 0] =
    //         ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].position.x;
    //     verts[stride + 1] =
    //         ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].position.y;
    //     verts[stride + 2] =
    //         ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].position.z;
    //     verts[stride + 3] =
    //         ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].normal.x;
    //     verts[stride + 4] =
    //         ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].normal.y;
    //     verts[stride + 5] =
    //         ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 + j].normal.z;
    //     verts[stride + 6] = ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3 +
    //     j].uv.x; verts[stride + 7] = ((vertex*)model->meshes->data[mesh_i].vertices->data)[i * 3
    //     + j].uv.y;
    //   }
    // }
    size_t static_vertex_size = 2 * sizeof(vec3) + sizeof(vec2);
    size_t skinned_vertex_size = 2 * sizeof(vec3) + sizeof(vec2) + 4 * sizeof(u32) + sizeof(vec4);
    size_t vertex_size = mesh.is_skinned ? skinned_vertex_size : static_vertex_size;

    TRACE("sizeof(vertex) -> %ld, vertex_size -> %ld\n", sizeof(vertex), vertex_size);
    if (mesh.is_skinned) {
      assert(vertex_size == (12 + 12 + 8 + 16 + 16));
    } else {
      assert(vertex_size == sizeof(vertex));
      assert(vertex_size == 8 * sizeof(float));
    }
    size_t buffer_size = vertex_size * num_vertices;
    u8* bytes = malloc(buffer_size);

    for (int i = 0; i < num_vertices; i++) {
      u8* p = bytes + vertex_size * i;
      u8* bone_data_offset = p + static_vertex_size;
      memcpy(p, &mesh.vertices->data[i], sizeof(vertex));
      if (mesh.is_skinned) {
        // printf("")
        memcpy(bone_data_offset, &mesh.vertex_bone_data->data[i], sizeof(vertex_bone_data));
      }
    }

    // 4. upload data
    glBufferData(GL_ARRAY_BUFFER, buffer_size, bytes, GL_STATIC_DRAW);

    // 5. cont. set mesh vertex layout
    glBindVertexArray(model->meshes->data[mesh_i].vao);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)0);
    glEnableVertexAttribArray(0);
    // normal vector attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // tex coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertex_size, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // skinning (optional)
    if (mesh.is_skinned) {
      glEnableVertexAttribArray(3);
      glVertexAttribIPointer(3, 4, GL_INT, vertex_size, (void*)(8 * sizeof(float)));

      glEnableVertexAttribArray(4);
      glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, vertex_size, (void*)(12 * sizeof(float)));
    }
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
  char* data = stbi_load(path, &width, &height, &num_channels, 0);  // STBI_rgb_alpha);
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

void dir_light_upload_uniforms(shader shader, directional_light* light) {
  uniform_vec3f(shader.program_id, "dirLight.direction", &light->direction);
  uniform_vec3f(shader.program_id, "dirLight.ambient", &light->ambient);
  uniform_vec3f(shader.program_id, "dirLight.diffuse", &light->diffuse);
  uniform_vec3f(shader.program_id, "dirLight.specular", &light->specular);
}

void point_light_upload_uniforms(shader shader, point_light* light, char index) {
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