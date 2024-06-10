#version 410

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

 uniform sampler2D texSampler;
// uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
  // outColor = texture(texSampler, fragTexCoord);  // vec4(fragTexCoord, 0.0);
  outColor = vec4(fragColor, 1.0);
}
