#version 410 core

uniform Camera {
  mat4 view;
  mat4 proj;
  vec4 viewPos;
} cam;

// Grid position are in xy clipped space
vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);
// normal vertice projection
void main() {
    gl_Position = cam.proj * cam.view * vec4(gridPlane[gl_VertexID].xyz, 1.0);
}