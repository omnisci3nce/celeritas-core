#include <assert.h>
#include "backend_opengl.h"
#include "log.h"
#include "mem.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"

#include <glad/glad.h>
#include <glfw3.h>

typedef struct OpenglCtx {
    GLFWwindow* window;
    arena pool_arena;
    GPU_CmdBuffer main_cmd_buffer;
    GPU_BackendPools gpu_pools;
    ResourcePools* resource_pools;
} OpenglCtx;

static OpenglCtx context;

bool GPU_Backend_Init(const char* window_name, struct GLFWwindow *window, struct ResourcePools* res_pools) {
    INFO("loading OpenGL backend");

    memset(&context, 0, sizeof(context));
    context.window = window;

    size_t pool_buffer_size = 1024 * 1024;
    context.pool_arena = arena_create(malloc(pool_buffer_size), pool_buffer_size);

    BackendPools_Init(&context.pool_arena, &context.gpu_pools);
    context.resource_pools = res_pools;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // glad: load all opengl function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      ERROR("Failed to initialise GLAD \n");
      return false;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    return true;
}

// All of these are no-ops in OpenGL
void GPU_Backend_Shutdown() {}
bool GPU_Device_Create(GPU_Device* out_device) { return true; }
void GPU_Device_Destroy(GPU_Device* device) {}
bool GPU_Swapchain_Create(GPU_Swapchain* out_swapchain) { return true; }
void GPU_Swapchain_Destroy(GPU_Swapchain* swapchain) {}
void GPU_CmdEncoder_Destroy(GPU_CmdEncoder* encoder) {}
void GPU_CmdEncoder_BeginRender(GPU_CmdEncoder* encoder, GPU_Renderpass* renderpass) {}
void GPU_CmdEncoder_EndRender(GPU_CmdEncoder* encoder) {}
GPU_CmdEncoder* GPU_GetDefaultEncoder() {
    return context.
}
void GPU_QueueSubmit(GPU_CmdBuffer* cmd_buffer) {}

GPU_Renderpass* GPU_Renderpass_Create(GPU_RenderpassDesc description) {
    // allocate new pass
    GPU_Renderpass* renderpass = Renderpass_pool_alloc(&context.gpu_pools.renderpasses, NULL);
    renderpass->description = description;

    if (!description.default_framebuffer) {
        // If we're not using the default framebuffer we need to generate a new one
        GLuint gl_fbo_id;
        glGenFramebuffers(1, &gl_fbo_id);
        renderpass->fbo = gl_fbo_id;
    } else {
        renderpass->fbo = OPENGL_DEFAULT_FRAMEBUFFER;
        assert(!description.has_color_target);
        assert(!description.has_depth_stencil);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, renderpass->fbo);

    if (description.has_color_target && !description.default_framebuffer) {
      GPU_Texture* colour_attachment = TEXTURE_GET(description.color_target);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                             colour_attachment->id, 0);
    }
    if (description.has_depth_stencil && !description.default_framebuffer) {
      GPU_Texture* depth_attachment = TEXTURE_GET(description.depth_stencil);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment->id,
                             0);
    }

    if (description.has_depth_stencil && !description.has_color_target) {
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // reset to default framebuffer

    return renderpass;
}

void GPU_Renderpass_Destroy(GPU_Renderpass* pass) { glDeleteFramebuffers(1, &pass->fbo); }

GPU_Pipeline* GPU_GraphicsPipeline_Create(GraphicsPipelineDesc description, GPU_Renderpass* renderpass) {
    GPU_Pipeline* pipeline = Pipeline_pool_alloc(&context.gpu_pools.pipelines, NULL);

    // Create shader program
    u32 shader_id = shader_create_separate(description.vs.filepath.buf, description.fs.filepath.buf);
    pipeline->shader_id = shader_id;

    // Vertex format
    pipeline->vertex_desc = description.vertex_desc;

    // Allocate uniform buffers if needed
    u32 ubo_count = 0;
    // printf("data layouts %d\n", description.data_layouts_count);
    for (u32 layout_i = 0; layout_i < description.data_layouts_count; layout_i++) {
      ShaderDataLayout sdl = description.data_layouts[layout_i].get_layout(NULL);
      TRACE("Got shader data layout %d's bindings! . found %d", layout_i, sdl.binding_count);

      for (u32 binding_j = 0; binding_j < sdl.binding_count; binding_j++) {
        u32 binding_id = binding_j;
        assert(binding_id < MAX_PIPELINE_UNIFORM_BUFFERS);
        ShaderBinding binding = sdl.bindings[binding_j];
        // Do I want Buffer vs Bytes?
        if (binding.kind == BINDING_BUFFER) {
          static u32 s_binding_point = 0;
          BufferHandle ubo_handle =
              GPU_BufferCreate(binding.data.bytes.size, BUFFER_UNIFORM, BUFFER_FLAG_GPU, NULL);  // no data right now
          pipeline->uniform_bindings[ubo_count++] = ubo_handle;
          GPU_Buffer* ubo_buf = BUFFER_GET(ubo_handle);

          i32 blockIndex = glGetUniformBlockIndex(pipeline->shader_id, binding.label);
          printf("Block index for %s: %d", binding.label, blockIndex);
          if (blockIndex < 0) {
            WARN("Couldn't retrieve block index for uniform block '%s'", binding.label);
          } else {
            // DEBUG("Retrived block index %d for %s", blockIndex, binding.label);
          }
          u32 blocksize;
          glGetActiveUniformBlockiv(pipeline->shader_id, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE,
                                    &blocksize);
          printf("\t with size %d bytes\n", blocksize);

          glBindBufferBase(GL_UNIFORM_BUFFER, s_binding_point, ubo_buf->id.ubo);
          if (blockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(pipeline->shader_id, blockIndex, s_binding_point);
          }
          ubo_buf->ubo_binding_point = s_binding_point++;
          ubo_buf->name = binding.label;
          assert(s_binding_point < GL_MAX_UNIFORM_BUFFER_BINDINGS);
        }
      }
    }
    pipeline->uniform_count = ubo_count;

    pipeline->renderpass = description.renderpass;
    pipeline->wireframe = description.wireframe;

    return pipeline;
}

void GraphicsPipeline_Destroy(GPU_Pipeline* pipeline) {}

GPU_CmdEncoder GPU_CmdEncoder_Create() {
  GPU_CmdEncoder encoder = { 0 };
  return encoder;
}


BufferHandle GPU_BufferCreate(u64 size, GPU_BufferType buf_type, GPU_BufferFlags flags, const void *data) {
    // "allocating" the cpu-side buffer struct
    BufferHandle handle;
    GPU_Buffer* buffer = Buffer_pool_alloc(&context.resource_pools, &handle);
    buffer->size = size;
    buffer->vao = 0;

    // Opengl buffer
    GLuint gl_buffer_id;
    glGenBuffers(1, &gl_buffer_id);

    GLenum gl_buf_type;
    GLenum gl_buf_usage = GL_STATIC_DRAW;

    switch (buf_type) {
      case BUFFER_UNIFORM:
        DEBUG("Creating Uniform buffer");
        gl_buf_type = GL_UNIFORM_BUFFER;
        /* gl_buf_usage = GL_DYNAMIC_DRAW; */
        buffer->id.ubo = gl_buffer_id;
        break;
      case BUFFER_DEFAULT:
      case BUFFER_VERTEX:
        DEBUG("Creating Vertex buffer");
        gl_buf_type = GL_ARRAY_BUFFER;
        buffer->id.vbo = gl_buffer_id;
        break;
      case BUFFER_INDEX:
        DEBUG("Creating Index buffer");
        gl_buf_type = GL_ELEMENT_ARRAY_BUFFER;
        buffer->id.ibo = gl_buffer_id;
        break;
      default:
        WARN("Unimplemented gpu_buffer_type provided %s", buffer_type_names[buf_type]);
        break;
    }
    // bind buffer
    glBindBuffer(gl_buf_type, gl_buffer_id);

    if (data) {
      TRACE("Upload data (%d bytes) as part of buffer creation", size);
      glBufferData(gl_buf_type, buffer->size, data, gl_buf_usage);
    } else {
      TRACE("Allocating but not uploading (%d bytes)", size);
      glBufferData(gl_buf_type, buffer->size, NULL, gl_buf_usage);
    }

    glBindBuffer(gl_buf_type, 0);

    return handle;
}

TextureHandle GPU_TextureCreate(TextureDesc desc, bool create_view, const void *data) {
    // "allocating" the cpu-side struct
    TextureHandle handle;
    GPU_Texture* texture = Texture_pool_alloc(&context.resource_pools->textures, &handle);
    DEBUG("Allocated texture with handle %d", handle.raw);

    GLuint gl_texture_id;
    glGenTextures(1, &gl_texture_id);
    texture->id = gl_texture_id;

    glBindTexture(GL_TEXTURE_2D, gl_texture_id);

    GLint internal_format =
        desc.format == TEXTURE_FORMAT_DEPTH_DEFAULT ? GL_DEPTH_COMPONENT : GL_RGB;
    GLenum format = desc.format == TEXTURE_FORMAT_DEPTH_DEFAULT ? GL_DEPTH_COMPONENT : GL_RGBA;
    GLenum data_type = desc.format == TEXTURE_FORMAT_DEPTH_DEFAULT ? GL_FLOAT : GL_UNSIGNED_BYTE;

    if (desc.format == TEXTURE_FORMAT_DEPTH_DEFAULT) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    } else {
      // set the texture wrapping parameters
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                      GL_REPEAT);  // set texture wrapping to GL_REPEAT (default wrapping method)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      // set texture filtering parameters
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    if (data) {
      glTexImage2D(GL_TEXTURE_2D, 0, internal_format, desc.extents.x, desc.extents.y, 0, format,
                   data_type, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      WARN("No image data provided");
      glTexImage2D(GL_TEXTURE_2D, 0, internal_format, desc.extents.x, desc.extents.y, 0, format,
                   data_type, NULL);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return handle;
}

void GPU_TextureDestroy(TextureHandle handle) {
    glDeleteTextures(1, &handle.raw);
}

// TODO: void GPU_TextureUpload(TextureHandle handle, size_t n_bytes, const void* data)

bool GPU_Backend_BeginFrame() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GPU_Backend_EndFrame() {
    glfwSwapBuffers(context.window);
}
