#version 430

layout( binding = 0) uniform Matrices {
  mat4 model;
  mat4 view;
  mat4 proj;
} ubo;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
  // gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
  gl_Position = projection * view * model * vec4(inPosition, 1.0);
  // gl_Position = vec4(inPosition, 1.0);
  fragColor = abs(inNormal);
  fragTexCoord = inTexCoords;
}
