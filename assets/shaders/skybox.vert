#version 410 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

uniform Camera {
  mat4 view;
  mat4 proj;
  vec4 viewPos;
} cam;

out vec3 TexCoords;

void main() {
    TexCoords = inPosition;
    gl_Position = cam.proj * cam.view * vec4(inPosition, 1.0);
}
