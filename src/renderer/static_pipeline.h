#pragma once
#include "defines.h"
#include "maths_types.h"
#include "ral.h"
#include "ral_types.h"
#include "render_types.h"

typedef struct mvp_uniforms {
  mat4 model;
  mat4 view;
  mat4 projection;
} mvp_uniforms;
typedef struct my_shader_bind_group {
  mvp_uniforms mvp;
} my_shader_bind_group;

static shader_data_layout mvp_uniforms_layout(void* data) {
  my_shader_bind_group* d = (my_shader_bind_group*)data;
  bool has_data = data != NULL;

  shader_binding b1 = { .label = "mvp_uniforms",
                        .type = SHADER_BINDING_BYTES,
                        .stores_data = has_data,
                        .data = { .bytes = { .size = sizeof(mvp_uniforms) } } };

  if (has_data) {
    b1.data.bytes.data = &d->mvp;
  }
  return (shader_data_layout){ .name = "global_ubo", .bindings = { b1 }, .bindings_count = 1 };
}
