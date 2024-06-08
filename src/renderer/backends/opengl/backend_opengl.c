#include <stddef.h>
#include "colours.h"
#include "opengl_helpers.h"
#include "ral_types.h"
#define CEL_REND_BACKEND_OPENGL
#if defined(CEL_REND_BACKEND_OPENGL)
#include <stdlib.h>
#include "camera.h"

#include "backend_opengl.h"
#include "defines.h"
#include "file.h"
#include "log.h"
#include "maths_types.h"
#include "ral.h"

#include <glad/glad.h>
#include <glfw3.h>

typedef struct opengl_context {
  GLFWwindow* window;
  arena pool_arena;
  gpu_cmd_encoder command_buffer;
  gpu_backend_pools gpu_pools;
  struct resource_pools* resource_pools;
} opengl_context;

static opengl_context context;

struct GLFWwindow;

size_t vertex_attrib_size(vertex_attrib_type attr);

bool gpu_backend_init(const char* window_name, struct GLFWwindow* window) {
  INFO("loading OpenGL backend");

  memset(&context, 0, sizeof(opengl_context));
  context.window = window;

  size_t pool_buffer_size = 1024 * 1024;
  context.pool_arena = arena_create(malloc(pool_buffer_size), pool_buffer_size);

  backend_pools_init(&context.pool_arena, &context.gpu_pools);
  context.resource_pools = malloc(sizeof(struct resource_pools));
  resource_pools_init(&context.pool_arena, context.resource_pools);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // glad: load all opengl function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    ERROR("Failed to initialise GLAD \n");
    return false;
  }

  glEnable(GL_DEPTH_TEST);

  return true;
}

void gpu_backend_shutdown() {}

bool gpu_device_create(gpu_device* out_device) {}
void gpu_device_destroy() {}

// --- Render Pipeline
gpu_pipeline* gpu_graphics_pipeline_create(struct graphics_pipeline_desc description) {
  gpu_pipeline* pipeline = pipeline_pool_alloc(&context.gpu_pools.pipelines, NULL);

  // Create shader program
  u32 shader_id = shader_create_separate(description.vs.filepath.buf, description.fs.filepath.buf);
  pipeline->shader_id = shader_id;

  // Vertex
  u32 vao;
  glGenVertexArrays(1, &vao);
  pipeline->vao = vao;

  // Attributes
  u32 attr_count = description.vertex_desc.attributes_count;
  printf("N attributes %d\n", attr_count);
  u64 offset = 0;
  size_t vertex_size = description.vertex_desc.stride;
  for (u32 i = 0; i < description.vertex_desc.attributes_count; i++) {
    opengl_vertex_attr format = format_from_vertex_attr(description.vertex_desc.attributes[i]);
    glVertexAttribPointer(i, format.count, format.data_type, GL_FALSE, vertex_size, (void*)offset);
    size_t this_offset = format.count * vertex_attrib_size(description.vertex_desc.attributes[i]);
    printf("offset total %lld this attr %ld\n", offset, this_offset);
    offset += this_offset;
  }

  // Allocate uniform buffers if needed
  printf("data layouts %d\n", description.data_layouts_count);
  for (u32 layout_i = 0; layout_i < description.data_layouts_count; layout_i++) {
    shader_data_layout sdl = description.data_layouts[layout_i].shader_data_get_layout(NULL);
    TRACE("Got shader data layout %d's bindings! . found %d", layout_i, sdl.bindings_count);

    for (u32 binding_j = 0; binding_j < sdl.bindings_count; binding_j++) {
    }
  }

  return pipeline;
}
void gpu_pipeline_destroy(gpu_pipeline* pipeline) {}

// --- Renderpass
gpu_renderpass* gpu_renderpass_create(const gpu_renderpass_desc* description) {}
void gpu_renderpass_destroy(gpu_renderpass* pass) {}

// --- Swapchain
bool gpu_swapchain_create(gpu_swapchain* out_swapchain) {}
void gpu_swapchain_destroy(gpu_swapchain* swapchain) {}

// --- Command buffer
gpu_cmd_encoder gpu_cmd_encoder_create() {
  gpu_cmd_encoder encoder = { 0 };
  return encoder;
}
void gpu_cmd_encoder_destroy(gpu_cmd_encoder* encoder) {}
void gpu_cmd_encoder_begin(gpu_cmd_encoder encoder) {}
void gpu_cmd_encoder_begin_render(gpu_cmd_encoder* encoder, gpu_renderpass* renderpass) {
  rgba clear_colour = STONE_900;
  glClearColor(clear_colour.r, clear_colour.g, clear_colour.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void gpu_cmd_encoder_end_render(gpu_cmd_encoder* encoder) {}
void gpu_cmd_encoder_begin_compute() {}
gpu_cmd_encoder* gpu_get_default_cmd_encoder() { return &context.command_buffer; }

/** @brief Finish recording and return a command buffer that can be submitted to a queue */
gpu_cmd_buffer gpu_cmd_encoder_finish(gpu_cmd_encoder* encoder) {}

void gpu_queue_submit(gpu_cmd_buffer* buffer) {}

// --- Data copy commands
/** @brief Copy data from one buffer to another */
void encode_buffer_copy(gpu_cmd_encoder* encoder, buffer_handle src, u64 src_offset,
                        buffer_handle dst, u64 dst_offset, u64 copy_size) {}
/** @brief Upload CPU-side data as array of bytes to a GPU buffer */
void buffer_upload_bytes(buffer_handle gpu_buf, bytebuffer cpu_buf, u64 offset, u64 size) {}

/** @brief Copy data from buffer to buffer using a one time submit command buffer and a wait */
void copy_buffer_to_buffer_oneshot(buffer_handle src, u64 src_offset, buffer_handle dst,
                                   u64 dst_offset, u64 copy_size) {}
/** @brief Copy data from buffer to an image using a one time submit command buffer */
void copy_buffer_to_image_oneshot(buffer_handle src, texture_handle dst) {}

// --- Render commands
void encode_bind_pipeline(gpu_cmd_encoder* encoder, pipeline_kind kind, gpu_pipeline* pipeline) {
  // In OpenGL this is more or less equivalent to just setting the shader
  glUseProgram(pipeline->shader_id);
  glBindVertexArray(pipeline->vao);
}
void encode_bind_shader_data(gpu_cmd_encoder* encoder, u32 group, shader_data* data) {
  shader_data_layout sdl = data->shader_data_get_layout(data->data);
  size_t binding_count = sdl.bindings_count;

  for (u32 i = 0; i < sdl.bindings_count; i++) {
    shader_binding binding = sdl.bindings[i];
  }
}
void encode_set_default_settings(gpu_cmd_encoder* encoder) {}
void encode_set_vertex_buffer(gpu_cmd_encoder* encoder, buffer_handle buf) {}
void encode_set_index_buffer(gpu_cmd_encoder* encoder, buffer_handle buf) {}
void encode_draw(gpu_cmd_encoder* encoder) {}
void encode_draw_indexed(gpu_cmd_encoder* encoder, u64 index_count) {}
void encode_clear_buffer(gpu_cmd_encoder* encoder, buffer_handle buf) {}

// --- Buffers
buffer_handle gpu_buffer_create(u64 size, gpu_buffer_type buf_type, gpu_buffer_flags flags,
                                const void* data) {
  GLuint gl_buffer_id;
  glGenBuffers(1, &gl_buffer_id);

  GLenum gl_buf_type;

  switch (buf_type) {
    case CEL_BUFFER_UNIFORM:
      gl_buf_type = GL_UNIFORM_BUFFER;
      break;
    case CEL_BUFFER_DEFAULT:
    case CEL_BUFFER_VERTEX:
      gl_buf_type = GL_ARRAY_BUFFER;
      break;
    case CEL_BUFFER_INDEX:
      gl_buf_type = GL_ELEMENT_ARRAY_BUFFER;
      break;
    default:
      WARN("Unimplemented gpu_buffer_type provided %s", buffer_type_names[buf_type]);
      break;
  }
  // bind buffer
  glBindBuffer(gl_buf_type, gl_buffer_id);

  // "allocating" the cpu-side buffer struct
  buffer_handle handle;
  gpu_buffer* buffer = buffer_pool_alloc(&context.resource_pools->buffers, &handle);
  buffer->size = size;

  if (data) {
    TRACE("Upload data (%d bytes) as part of buffer creation", size);
    glBufferData(gl_buf_type, buffer->size, data, GL_STATIC_DRAW);
  }

  return handle;
}

void gpu_buffer_destroy(buffer_handle buffer) {}
void gpu_buffer_upload(const void* data) {}

// Textures
/** @brief Create a new GPU texture resource.
 *  @param create_view creates a texture view (with same dimensions) at the same time
 *  @param data if not NULL then the data stored at the pointer will be uploaded to the GPU texture
 *  @note automatically creates a sampler for you */
texture_handle gpu_texture_create(texture_desc desc, bool create_view, const void* data) {}
void gpu_texture_destroy(texture_handle) {}
void gpu_texture_upload(texture_handle texture, const void* data) {}

// --- Vertex formats
bytebuffer vertices_as_bytebuffer(arena* a, vertex_format format, vertex_darray* vertices) {}

// --- TEMP
bool gpu_backend_begin_frame() { return true; }
void gpu_backend_end_frame() { glfwSwapBuffers(context.window); }
void gpu_temp_draw(size_t n_verts) {}

u32 shader_create_separate(const char* vert_shader, const char* frag_shader) {
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

// /** @brief Internal backend state */
// typedef struct opengl_state {
// } opengl_state;

// bool gfx_backend_init(renderer *ren) {
//   INFO("loading OpenGL backend");

//   // glfwInit(); // Already handled in `renderer_init`
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
//   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

//   // glad: load all OpenGL function pointers
//   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
//     ERROR("Failed to initialise GLAD \n");

//     return false;
//   }

//   glEnable(GL_DEPTH_TEST);

//   opengl_state *internal = malloc(sizeof(opengl_state));
//   ren->backend_context = (void *)internal;

//   return true;
// }

// void gfx_backend_draw_frame(renderer *ren, camera *cam, mat4 model, texture *tex) {}

// void gfx_backend_shutdown(renderer *ren) {}

void uniform_vec3f(u32 program_id, const char* uniform_name, vec3* value) {
  glUniform3fv(glGetUniformLocation(program_id, uniform_name), 1, &value->x);
}
void uniform_f32(u32 program_id, const char* uniform_name, f32 value) {
  glUniform1f(glGetUniformLocation(program_id, uniform_name), value);
}
void uniform_i32(u32 program_id, const char* uniform_name, i32 value) {
  glUniform1i(glGetUniformLocation(program_id, uniform_name), value);
}
void uniform_mat4f(u32 program_id, const char* uniform_name, mat4* value) {
  glUniformMatrix4fv(glGetUniformLocation(program_id, uniform_name), 1, GL_FALSE, value->data);
}

// void clear_screen(vec3 colour) {
//   glClearColor(colour.x, colour.y, colour.z, 1.0f);
//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// }

// void texture_data_upload(texture *tex) {
//   printf("Texture name %s\n", tex->name);
//   TRACE("Upload texture data");
//   u32 texture_id;
//   glGenTextures(1, &texture_id);
//   glBindTexture(GL_TEXTURE_2D, texture_id);
//   tex->texture_id = texture_id;

//   // set the texture wrapping parameters
//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
//                   GL_REPEAT);  // set texture wrapping to GL_REPEAT (default wrapping method)
//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//   // set texture filtering parameters
//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->width, tex->height, 0, tex->channel_type,
//                GL_UNSIGNED_BYTE, tex->image_data);
//   glGenerateMipmap(GL_TEXTURE_2D);
//   DEBUG("Freeing texture image data after uploading to GPU");
//   // stbi_image_free(tex->image_data);  // data is on gpu now so we dont need it around
// }

// void bind_texture(shader s, texture *tex, u32 slot) {
//   // printf("bind texture slot %d with texture id %d \n", slot, tex->texture_id);
//   glActiveTexture(GL_TEXTURE0 + slot);
//   glBindTexture(GL_TEXTURE_2D, tex->texture_id);
// }

// void bind_mesh_vertex_buffer(void *_backend, mesh *mesh) { glBindVertexArray(mesh->vao); }

// static inline GLenum to_gl_prim_topology(enum cel_primitive_topology primitive) {
//   switch (primitive) {
//     case CEL_PRIMITIVE_TOPOLOGY_TRIANGLE:
//       return GL_TRIANGLES;
//     case CEL_PRIMITIVE_TOPOLOGY_POINT:
//     case CEL_PRIMITIVE_TOPOLOGY_LINE:
//     case CEL_PRIMITIVE_TOPOLOGY_LINE_STRIP:
//     case CEL_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
//     case CEL_PRIMITIVE_TOPOLOGY_COUNT:
//       break;
//   }
// }

// void draw_primitives(cel_primitive_topology primitive, u32 start_index, u32 count) {
//   u32 gl_primitive = to_gl_prim_topology(primitive);
//   glDrawArrays(gl_primitive, start_index, count);
// }

// shader shader_create_separate(const char *vert_shader, const char *frag_shader) {
//   INFO("Load shaders at %s and %s", vert_shader, frag_shader);
//   int success;
//   char info_log[512];

//   u32 vertex = glCreateShader(GL_VERTEX_SHADER);
//   const char *vertex_shader_src = string_from_file(vert_shader);
//   if (vertex_shader_src == NULL) {
//     ERROR("EXIT: couldnt load shader");
//     exit(-1);
//   }
//   glShaderSource(vertex, 1, &vertex_shader_src, NULL);
//   glCompileShader(vertex);
//   glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
//   if (!success) {
//     glGetShaderInfoLog(vertex, 512, NULL, info_log);
//     printf("%s\n", info_log);
//     ERROR("EXIT: vertex shader compilation failed");
//     exit(-1);
//   }

//   // fragment shader
//   u32 fragment = glCreateShader(GL_FRAGMENT_SHADER);
//   const char *fragment_shader_src = string_from_file(frag_shader);
//   if (fragment_shader_src == NULL) {
//     ERROR("EXIT: couldnt load shader");
//     exit(-1);
//   }
//   glShaderSource(fragment, 1, &fragment_shader_src, NULL);
//   glCompileShader(fragment);
//   glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
//   if (!success) {
//     glGetShaderInfoLog(fragment, 512, NULL, info_log);
//     printf("%s\n", info_log);
//     ERROR("EXIT: fragment shader compilation failed");
//     exit(-1);
//   }

//   u32 shader_prog;
//   shader_prog = glCreateProgram();

//   glAttachShader(shader_prog, vertex);
//   glAttachShader(shader_prog, fragment);
//   glLinkProgram(shader_prog);
//   glDeleteShader(vertex);
//   glDeleteShader(fragment);
//   free((char *)vertex_shader_src);
//   free((char *)fragment_shader_src);

//   shader s = { .program_id = shader_prog };
//   return s;
// }

// void set_shader(shader s) { glUseProgram(s.program_id); }

#endif
