#version 410 core

out vec4 FragColor;

in vec4 Color;
in vec2 TexCoord;

uniform sampler2D TextureSlot1;

void main() {
    vec4 tex_color = texture(TextureSlot1, TexCoord);
    FragColor = Color * tex_color;
}