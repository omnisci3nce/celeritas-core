#version 410 core
// Inputs
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

// Uniform block
layout (std140, binding = 0) uniform MatrixBlock {
  mat4 model;
  mat4 view;
  mat4 projection;
  mat4 lightSpaceMatrix;
};

// Output
out VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
  vec4 FragPosLightSpace;
} vs_out;

void main() {
  vs_out.FragPos = vec3(model * vec4(inPos, 1.0));
  vs_out.Normal = inNormal;
  vs_out.TexCoords = inTexCoords;
  vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
  gl_Position =  projection * view * model * vec4(inPos, 1.0);
}