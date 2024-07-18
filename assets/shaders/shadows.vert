#version 410 core

// Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

// Uniforms
uniform ShadowUniforms {
  mat4 model;
  mat4 lightSpace;
} uniforms;

void main() {
  gl_Position = uniforms.lightSpace * uniforms.model * vec4(inPosition, 1.0);
}
