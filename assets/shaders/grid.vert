#version 410 core

uniform Camera {
  mat4 view;
  mat4 proj;
  vec4 viewPos;
} cam;

out vec3 nearPoint;
out vec3 farPoint;
out float near;
out float far;
out mat4 fragView;
out mat4 fragProj;

// Grid position are in xy clipped space
vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

// normal vertex projection
void main() {
    // gl_Position = cam.proj * cam.view * vec4(gridPlane[gl_VertexID].xyz, 1.0);
    vec3 p = gridPlane[gl_VertexID].xyz;
    nearPoint = UnprojectPoint(p.x, p.y, -1.0, cam.view, cam.proj).xyz; // unprojecting on the near plane
    farPoint = UnprojectPoint(p.x, p.y, 1.0, cam.view, cam.proj).xyz; // unprojecting on the far plane

    fragView = cam.view;
    fragProj = cam.proj;
    near = 0.01;
    far = 100.0;

    gl_Position = vec4(p, 1.0); // using directly the clipped coordinates
}