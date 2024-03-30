#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec3 in_position;

layout(location = 0) out vec4 out_colour;

void main() { out_colour = vec4(in_normal, 1.0); }