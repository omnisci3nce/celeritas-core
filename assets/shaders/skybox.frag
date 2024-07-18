#version 410 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube cubeMap;

void main() {
    FragColor = texture(cubeMap, TexCoords);
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
