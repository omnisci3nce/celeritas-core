#version 430 core

struct Uniforms {
  mat4 model;
  mat4 view;
  mat4 projection;
};

uniform Uniforms ubo;

// Inputs
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;
// uniform mat4 lightSpaceMatrix;

// Output
out VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
  // vec4 FragPosLightSpace;
  vec4 Color;
} vs_out;

void main() {
  vs_out.FragPos = vec3(ubo.model * vec4(inPos, 1.0));
  vs_out.Normal = inNormal;
  vs_out.TexCoords = inTexCoords;
  // vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
  vs_out.Color = vec4(1.0);
  gl_Position =  ubo.projection * ubo.view * ubo.model * vec4(inPos, 1.0);
}
