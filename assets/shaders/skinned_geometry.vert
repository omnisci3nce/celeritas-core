#version 410 core

// Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in ivec4 inBoneIndices;
layout(location = 4) in vec4 inWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCES = 4;

uniform AnimData {
    mat4 boneMatrices[MAX_BONES];
} anim;

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
// out vec4 dbgcolor;

void main() {
    mat4 skinMatrix =
        inWeights.x * anim.boneMatrices[int(inBoneIndices.x)] +
            inWeights.y * anim.boneMatrices[int(inBoneIndices.y)] +
            inWeights.z * anim.boneMatrices[int(inBoneIndices.z)] +
            inWeights.w * anim.boneMatrices[int(inBoneIndices.w)];

    vec4 totalPosition = skinMatrix * vec4(inPosition, 1.0);

    fragWorldPos = vec3(model.inner * totalPosition);
    fragNormal = mat3(transpose(inverse(model.inner))) * inNormal; // world-space normal
    fragTexCoords = inTexCoords;

    viewPos = cam.viewPos;

    gl_Position = cam.proj * cam.view * model.inner * vec4(inPosition, 1.0);
    // gl_Position = cam.proj * cam.view * model.inner * totalPosition;
}
