#include "render.h"

#include <glad/glad.h>
#include <glfw3.h>

#include "maths.h"
#include "defines.h"
#include "log.h"
#include "render_types.h"
#include "render_backend.h"

void draw_mesh_shadow(renderer* ren, mesh* mesh);

typedef struct draw_ctx {
  model_darray* models;
  renderer* ren;
  camera* cam;
  scene* scene;
} draw_ctx;

bool draw_shadow_scene_node(transform_node* node, void* ctx_data) {
  if (!node || !node->parent) return true;
  draw_ctx* ctx = ctx_data;
  model* m = &ctx->models->data[node->model.raw];

  // FIXME: Replace without a call that leaks OpenGL
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, ctx->ren->depth_texture.raw);

  // Upload model matrix
  uniform_mat4f(ctx->ren->shadow_map_depth.program_id, "model", &node->world_matrix_tf);
  // TODO: Equivalent of this - draw_model(ctx->ren, ctx->cam, m, &node->world_matrix_tf, ctx->scene);
  for (int i = 0; i < mesh_darray_len(m->meshes); i++) {
    draw_mesh_shadow(ctx->ren, &m->meshes->data[i]);
  }

  return true;
}

void draw_shadows(renderer* ren, arena* frame, model_darray* models, vec3 light_pos,
                  transform_hierarchy* tfh, scene* scene) {
 // Calculate camera representing hte light's coordinate frame
  f32 near_plane = 1.0, far_plane = 7.5;
  mat4 light_projection = mat4_orthographic(-10.0, 10.0, -10.0, 10.0, near_plane, far_plane);
  mat4 light_view = mat4_look_at(light_pos, VEC3_ZERO, VEC3_Y);
  mat4 light_space_matrix = mat4_mult(light_view, light_projection);
  // Render scene from light's perspective
  // upload the light space transform matrix to our shader
  set_shader(ren->shadow_map_depth);
  uniform_mat4f(ren->shadow_map_depth.program_id, "lightSpaceMatrix", &light_space_matrix);

  draw_ctx* ctx = arena_alloc(frame, sizeof(draw_ctx));
  ctx->models = models;
  ctx->ren = ren;
  ctx->cam = NULL; // unused
  ctx->scene = scene;

  // Kickoff scene draw
  transform_hierarchy_dfs(transform_hierarchy_root_node(tfh), draw_shadow_scene_node, true,ctx);
}

void draw_mesh_shadow(renderer* ren, mesh* mesh) {
  bind_mesh_vertex_buffer(ren->backend_state, mesh);
  u32 num_vertices = vertex_darray_len(mesh->vertices);
  draw_primitives(CEL_PRIMITIVE_TOPOLOGY_TRIANGLE, 0, num_vertices);
}