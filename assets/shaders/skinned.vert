#version 410 core
// Inputs
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 3) in ivec4 inBoneIndices;
layout (location = 4) in vec4 inWeights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCES = 4;
uniform mat4 finalBoneMatrices[MAX_BONES];

// Output
out VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
  vec4 FragPosLightSpace;
} vs_out;

void main() {
  vec4 totalPosition = vec4(0.0f);
  for(int i = 0 ; i < MAX_BONE_INFLUENCES ; i++) {
      if(inBoneIndices[i] == -1) 
          continue;
      if(inBoneIndices[i] >=MAX_BONES) 
      {
          totalPosition = vec4(inPos,1.0f);
          break;
      }
      vec4 localPosition = finalBoneMatrices[inBoneIndices[i]] * vec4(inPos,1.0f);
      totalPosition += localPosition * inWeights[i];
      vec3 localNormal = mat3(finalBoneMatrices[inBoneIndices[i]]) * inNormal;
      vs_out.Normal = localNormal;
  }

  vs_out.FragPos = vec3(model * vec4(inPos, 1.0));
  // vs_out.Normal = inNormal;
  vs_out.TexCoords = inTexCoords;
  vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
  gl_Position =  projection * view * model * totalPosition;
}