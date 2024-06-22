#version 410 core
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = inTexCoords;
    vec2 xy = inPosition.xz;
    gl_Position = vec4(xy, 0.0, 1.0);
}
