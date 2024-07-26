#version 410 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

uniform Camera {
  mat4 view;
  mat4 proj;
  vec4 viewPos;
} cam;

out vec4 Color;
out vec2 TexCoord;

void main() {
  vec3 position = vec3(inPosition.x, inPosition.y / 2.0, inPosition.z);
  gl_Position =  cam.proj * cam.view * vec4(position, 1.0);

  Color = vec4(inPosition.y / 100.0);
  TexCoord = inTexCoords;
}