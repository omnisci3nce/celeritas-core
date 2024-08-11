#version 410 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

uniform Camera {
    mat4 view;
    mat4 proj;
    vec4 viewPos;
} cam;

uniform ImmUniforms {
    mat4 model;
    vec4 colour;
} imm;

out vec4 aColour;

void main() {
    aColour = imm.colour;
    gl_Position = cam.proj * cam.view * imm.model * vec4(inPosition, 1.0);
}
