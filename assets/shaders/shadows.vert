#version 410 core

// Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

// Uniforms
uniform Model {
  mat4 mat;
};

uniform LightSpace {
  mat4 mat;
};

void main() {
  gl_Position = LightSpace.mat * Model.mat * vec4(inPosition, 1.0);
}
