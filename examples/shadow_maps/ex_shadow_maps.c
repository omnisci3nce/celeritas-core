#include "celeritas.h"
#include "input.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "primitives.h"
#include "ral.h"
#include "ral_types.h"
#include "render.h"
#include "render_types.h"
#include "renderpasses.h"

extern core g_core;

// Scene / light setup
vec3 light_position = { -2, 4, -1 };
mesh s_scene[5];
transform s_transforms[5];

void draw_scene();
void switch_view();

enum active_view {
  SceneView = 0,
  LightView,
  DebugQuad,
};

// Define the shader data
typedef struct mvp_uniforms {
  mat4 model;
  mat4 view;
  mat4 projection;
} mvp_uniforms;
typedef struct my_shader_bind_group {
  mvp_uniforms mvp;
  texture_handle tex;
} my_shader_bind_group;

// We also must create a function that knows how to return a `shader_data_layout`
shader_data_layout mvp_uniforms_layout(void* data) {
  my_shader_bind_group* d = (my_shader_bind_group*)data;
  bool has_data = data != NULL;

  shader_binding b1 = { .label = "Matrices",
                        .type = SHADER_BINDING_BYTES,
                        .stores_data = has_data,
                        .data = { .bytes = { .size = sizeof(mvp_uniforms) } } };

  shader_binding b2 = { .label = "texSampler",
                        .type = SHADER_BINDING_TEXTURE,
                        .stores_data = has_data };
  if (has_data) {
    b1.data.bytes.data = &d->mvp;
    b2.data.texture.handle = d->tex;
  }
  return (shader_data_layout){ .name = "global_ubo", .bindings = { b1, b2 }, .bindings_count = 2 };
}

int main() {
  // Stuff that gets changed during program
  enum active_view current_view = SceneView;

  core_bringup();
  arena scratch = arena_create(malloc(1024 * 1024), 1024 * 1024);

  vec3 camera_pos = vec3(5, 5, 5);
  vec3 camera_front = vec3_normalise(vec3_negate(camera_pos));
  camera cam = camera_create(camera_pos, camera_front, VEC3_Y, deg_to_rad(45.0));
  // TEMP
  shader_data mvp_uniforms_data = { .data = NULL, .shader_data_get_layout = &mvp_uniforms_layout };

  gpu_renderpass_desc pass_description = { .default_framebuffer = true };
  gpu_renderpass* renderpass = gpu_renderpass_create(&pass_description);

  str8 vert_path, frag_path;
#ifdef CEL_REND_BACKEND_OPENGL
  vert_path = str8lit("assets/shaders/cube.vert");
  frag_path = str8lit("assets/shaders/cube.frag");
#else
  vert_path = str8lit("build/linux/x86_64/debug/cube.vert.spv");
  frag_path = str8lit("build/linux/x86_64/debug/cube.frag.spv");
#endif
  str8_opt vertex_shader = str8_from_file(&scratch, vert_path);
  str8_opt fragment_shader = str8_from_file(&scratch, frag_path);
  if (!vertex_shader.has_value || !fragment_shader.has_value) {
    ERROR_EXIT("Failed to load shaders from disk")
  }

  struct graphics_pipeline_desc pipeline_description = {
    .debug_name = "Basic Pipeline",
    .vertex_desc = static_3d_vertex_description(),
    .data_layouts = { mvp_uniforms_data },
    .data_layouts_count = 1,
    .vs = { .debug_name = "Cube Vertex Shader",
            .filepath = vert_path,
            .code = vertex_shader.contents,
            .is_spirv = true },
    .fs = { .debug_name = "Cube Fragment Shader",
            .filepath = frag_path,
            .code = fragment_shader.contents,
            .is_spirv = true },
    .renderpass = renderpass,
    .wireframe = false,
    .depth_test = false
  };
  gpu_pipeline* gfx_pipeline = gpu_graphics_pipeline_create(pipeline_description);

  // Textures
  texture_data tex_data = texture_data_load("assets/textures/texture.jpg", false);
  texture_handle texture = texture_data_upload(tex_data, true);
  texture_handle white_tex =
      texture_data_upload(texture_data_load("assets/textures/white1x1.png", false), true);

  // END TEMP

  ren_shadowmaps shadows = { .width = 1000, .height = 1000 };
  ren_shadowmaps_init(&shadows);

  // Set up the scene
  // We want:
  // 1. a ground plane
  // 2. lights
  // 3. some boxes
  for (int i = 0; i < 4; i++) {
    geometry_data geo = geo_create_cuboid(f32x3(1, 1, 1));
    s_scene[i] = mesh_create(&geo, true);
    s_transforms[i] = transform_create(vec3(-2 + (i * 1.2), 0, 0), quat_ident(), 1.0);
  }
  geometry_data plane = geo_create_plane(f32x2(20, 20));
  s_scene[4] = mesh_create(&plane, true);
  s_transforms[4] = transform_create(vec3(0, -3, 0), quat_ident(), 1.0);

  geometry_data quad_geo = geo_create_plane(f32x2(2, 2));

  // HACK: Swap vertices to make it face us
  vertex top0 = quad_geo.vertices->data[0];
  quad_geo.vertices->data[0] = quad_geo.vertices->data[2];
  quad_geo.vertices->data[2] = top0;
  vertex top1 = quad_geo.vertices->data[1];
  quad_geo.vertices->data[1] = quad_geo.vertices->data[3];
  quad_geo.vertices->data[3] = top1;

  mesh quad = mesh_create(&quad_geo, true);

  // Main loop
  while (!should_exit(&g_core)) {
    input_update(&g_core.input);
    printf("Frame\n");

    if (key_just_released(KEYCODE_L)) {
      current_view = (current_view + 1) % 3;
    }
    if (key_just_released(KEYCODE_R)) {
      // TODO: gpu_pipeline_reload_shaders(gfx_pipeline);
    }

    if (!gpu_backend_begin_frame()) {
      continue;
    }
    gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
    gpu_cmd_encoder_begin(*enc);

    // Shadow draw
    gpu_cmd_encoder_begin_render(enc, shadows.rpass);
    printf("Here\n");

    // calculations
    f32 near_plane = 1.0, far_plane = 10.0;
    mat4 light_projection = mat4_orthographic(-10.0, 10.0, -10.0, 10.0, near_plane, far_plane);
    mat4 light_view = mat4_look_at(light_position, VEC3_ZERO, VEC3_Y);
    mat4 light_space_matrix = mat4_mult(light_view, light_projection);
    lightspace_tf_uniform lsu = { .lightSpaceMatrix = light_space_matrix };

    encode_bind_pipeline(enc, PIPELINE_GRAPHICS, shadows.static_pipeline);

    shader_data lightspace_data = { .data = NULL,
                                    .shader_data_get_layout = &lightspace_uniform_layout };
    lightspace_data.data = &lsu;
    encode_bind_shader_data(enc, 0, &lightspace_data);

    draw_scene();
    printf("Here\n");

    gpu_cmd_encoder_end_render(enc);
    // End

    // Debug quad
    if (current_view == DebugQuad) {
      gpu_cmd_encoder_begin_render(enc, shadows.debug_quad->renderpass);

      encode_bind_pipeline(enc, PIPELINE_GRAPHICS, shadows.debug_quad);
      debug_quad_uniform dqu = { .depthMap = shadows.depth_tex };
      shader_data debug_quad_data = { .data = &dqu, .shader_data_get_layout = debug_quad_layout };
      encode_bind_shader_data(enc, 0, &debug_quad_data);
      encode_set_vertex_buffer(enc, quad.vertex_buffer);
      encode_set_index_buffer(enc, quad.index_buffer);
      encode_draw_indexed(enc, quad.geometry->indices->len);

      gpu_cmd_encoder_end_render(enc);
    }

    // Basic draw
    if (current_view == SceneView || current_view == LightView) {
      gpu_cmd_encoder_begin_render(enc, renderpass);
      encode_bind_pipeline(enc, PIPELINE_GRAPHICS, gfx_pipeline);

      mat4 view, proj;
      if (current_view == SceneView) {
        camera_view_projection(&cam, 1000, 1000, &view, &proj);
      } else {
        view = light_view;
        proj = light_projection;
      }
      for (int i = 0; i < 5; i++) {
        mat4 model = transform_to_mat(&s_transforms[i]);
        mvp_uniforms mvp_data = { .model = model, .view = view, .projection = proj };
        my_shader_bind_group shader_bind_data = { .mvp = mvp_data, .tex = texture };
        if (i == 4) {
          shader_bind_data.tex = white_tex;
        }
        mvp_uniforms_data.data = &shader_bind_data;
        encode_bind_shader_data(enc, 0, &mvp_uniforms_data);
        encode_set_vertex_buffer(enc, s_scene[i].vertex_buffer);
        encode_set_index_buffer(enc, s_scene[i].index_buffer);
        encode_draw_indexed(enc, s_scene[i].geometry->indices->len);
      }

      gpu_cmd_encoder_end_render(enc);
    }

    // END drawing
    gpu_cmd_buffer buf = gpu_cmd_encoder_finish(enc);
    gpu_queue_submit(&buf);
    gpu_backend_end_frame();
  }

  renderer_shutdown(&g_core.renderer);

  return 0;
}

void draw_scene() {
  gpu_cmd_encoder* enc = gpu_get_default_cmd_encoder();
  for (int i = 0; i < 5; i++) {
    mat4 model = transform_to_mat(&s_transforms[i]);
    model_uniform mu = { .model = model };
    shader_data model_data = { .data = &mu, .shader_data_get_layout = model_uniform_layout };
    encode_bind_shader_data(enc, 0, &model_data);
    printf("Here1\n");
    encode_set_vertex_buffer(enc, s_scene[i].vertex_buffer);
    printf("Here2\n");
    encode_set_index_buffer(enc, s_scene[i].index_buffer);
    printf("Here3\n");
    printf("num %ld \n", s_scene[i].geometry->indices->len);
    encode_draw_indexed(enc, s_scene[i].geometry->indices->len);
    printf("Here4\n");
  }
}
