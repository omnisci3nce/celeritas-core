#version 410 core

// Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

uniform Camera {
  mat4 view;
  mat4 proj;
  vec4 viewPos;
} cam;

uniform Model {
  mat4 inner;
} model;

// Outputs
layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoords;

out vec4 viewPos;

void main() {
  fragWorldPos = vec3(model.inner * vec4(inPosition, 1.0));
  fragNormal = mat3(transpose(inverse(model.inner))) * inNormal; // world-space normal
  fragTexCoords = inTexCoords;

  viewPos = cam.viewPos;

  gl_Position =  cam.proj * cam.view * model.inner * vec4(inPosition, 1.0);
}
