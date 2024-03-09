#version 410 core
out vec4 FragColor;

// A Blinn-Phong material with textures for diffuse and specular
// lighting maps and a numeric shininess factor.
struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
};

in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
  vec4 FragPosLightSpace;
} fs_in;

void main() {
  FragColor = vec4(1.0);
}