#version 410 core

// Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

// Uniforms
uniform Model {
  mat4 mat;
} model;

uniform LightSpace {
  mat4 mat;
} lightSpace;

void main() {
  gl_Position = lightSpace.mat * model.mat * vec4(inPosition, 1.0);
}
