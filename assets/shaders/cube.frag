#version 430

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
  // outColor = texture(texSampler, fragTexCoord);  // vec4(fragTexCoord, 0.0);
  outColor = vec4(1.0);
}
