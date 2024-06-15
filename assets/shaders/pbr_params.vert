#version 410

// Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

// Uniforms
uniform MVP_Matrices {
  mat4 model; // TODO: make model a push constant/raw uniform
  mat4 view;
  mat4 proj;
} mvp;

// Outputs
layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoords;

void main() {
  fragWorldPos = vec3(mvp.model * vec4(inPosition, 1.0));
  // TODO: fragNormal
  fragTexCoords = inTexCoords;

  gl_Position =  mvp.proj * mvp.view * mvp.model * vec4(inPosition, 1.0);
}
