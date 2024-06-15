#version 410

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

struct PointLight {
    vec3 position;
    vec3 color;
};

// --- Uniforms
// Lights data
#define NUM_POINT_LIGHTS 4
uniform Scene_Lights {
    vec3 viewPos;
    PointLight pointLights[NUM_POINT_LIGHTS];
} scene;
// Material properties
uniform PBR_Params {
  uniform vec3 albedo;
  uniform float metallic;
  uniform float roughness;
  uniform float ao;
} pbr;

const float PI = 3.14;

// Forward declarations
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

void main() {
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(scene.viewPos - WorldPos);

  // vec3 radiance = vec3(0.0); // denoted L in the radiance equation
  // for (int i = 0; i < 4; i++) {
  //   vec3 lightVec = normalize(pointLights[i].position - WorldPos);
  //   vec3 halfway = normalize(Normal + lightVec);
  //   float distance = length(pointLights[i].position - WorldPos);
  //   float attenuation = 1.0 / (distance * distance);
  //   vec3 radiance = pointLights[i].color * attenuation;

  //   vec3 F0 = vec3(0.04);
  //   F0      = mix(F0, albedo, metallic);
  //   vec3 F  = fresnelSchlick(max(dot(halfway, lightVec), 0.0), F0);
  // }

  FragColor = vec4(1.0);
}

/* The below are from https://learnopengl.com/PBR/Lighting */

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a      = roughness*roughness;
  float a2     = a*a;
  float NdotH  = max(dot(N, H), 0.0);
  float NdotH2 = NdotH*NdotH;

  float num   = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
