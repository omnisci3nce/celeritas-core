#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_coord;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_tex_coord;

layout(set = 0, binding = 0) uniform global_object_uniform {
  mat4 projection;
  mat4 view;
}
global_ubo;

layout(push_constant) uniform push_constants {
  mat4 model;  // 64 bytes
}
u_push_constants;

void main() {
  gl_Position = global_ubo.projection * global_ubo.view * u_push_constants.model *
                vec4(in_position.x, in_position.y, in_position.z, 1.0);
  out_position = in_position;
  out_normal = in_normal;
  out_tex_coord = in_tex_coord;
}