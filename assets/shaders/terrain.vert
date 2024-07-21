#version 410 core

layout(location = 0) in vec3 inPosition;

uniform Camera {
  mat4 view;
  mat4 proj;
  vec4 viewPos;
} cam;

void main() {
  gl_Position =  cam.proj * cam.view * vec4(inPosition, 1.0);
}