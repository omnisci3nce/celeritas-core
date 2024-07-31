#version 410 core

uniform Camera {
  mat4 view;
  mat4 proj;
  vec4 viewPos;
} cam;

void main() {
  gl_Position =  vec4(1.0);
}