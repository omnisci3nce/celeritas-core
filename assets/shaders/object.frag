#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_coord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 out_color;

void main() {
  // out_color = vec4(1.0);
  out_color = texture(texSampler, in_tex_coord);
}