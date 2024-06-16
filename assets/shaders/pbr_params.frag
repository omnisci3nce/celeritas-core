#version 410 core

out vec4 FragColor;

in vec3 fragWorldPos;
in vec3 fragNormal;
in vec2 fragTexCoords;

struct PointLight {
    vec4 position;
    vec4 color;
};

// --- Uniforms
// Material properties
uniform PBR_Params {
  uniform vec3 albedo;
  uniform float metallic;
  uniform float roughness;
  uniform float ao;
} pbr;
// Lights data
#define NUM_POINT_LIGHTS 4
uniform Scene_Lights {
    PointLight pointLights[NUM_POINT_LIGHTS];
    vec4 viewPos;
} scene;

const float PI = 3.14;

// Forward declarations
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

void main() {
  vec3 norm = normalize(fragNormal); // N
  vec3 N = norm;
  vec3 viewDir = normalize(vec3(scene.viewPos) - fragWorldPos); // V
  vec3 V = viewDir;

  vec3 F0 = vec3(0.04);
  F0      = mix(F0, pbr.albedo, pbr.metallic);

  vec3 Lo = vec3(0.0);
  for (int i = 0; i < 4; ++i) {
    vec3 lightVec = normalize(vec3(scene.pointLights[i].position) - fragWorldPos); // L
    vec3 L = lightVec;
    vec3 halfway = normalize(viewDir + lightVec); // H
    vec3 H = halfway;
    float distance = length(vec3(scene.pointLights[i].position) - fragWorldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = vec3(scene.pointLights[i].color) * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(norm, halfway, pbr.roughness);
    float G   = GeometrySmith(norm, viewDir, lightVec, pbr.roughness);
    // vec3 F    = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);
    vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    vec3 kS = F;

    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - pbr.metallic;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * pbr.albedo / PI + specular) * radiance * NdotL;
    // Lo += radiance;
  }

  vec3 ambient = vec3(0.03) * pbr.albedo * pbr.ao;
  vec3 color = ambient + Lo; //ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
   
   FragColor = vec4(color, 1.0);
  // FragColor = vec4(scene.pointLights[0].position);
  // FragColor = vec4(fragNormal, 1.0);
  // FragColor = vec4(pbr.metallic, pbr.roughness, pbr.ao, 1.0);
  // FragColor = scene.viewPos;
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
