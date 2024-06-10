#include <glfw3.h>

#include "animation.h"
#include "camera.h"
#include "core.h"
#include "loaders.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "render.h"
#include "render_types.h"

extern core g_core;

#define MODEL_GET(h) (model_pool_get(&g_core.models, h))

const vec3 pointlight_positions[4] = {
  { 0.7, 0.2, 2.0 },
  { 2.3, -3.3, -4.0 },
  { -4.0, 2.0, -12.0 },
  { 0.0, 0.0, -3.0 },
};
point_light point_lights[4];

int main() {
  double currentFrame = glfwGetTime();
  double lastFrame = currentFrame;
  double deltaTime;

  core_bringup();

  model_handle helmet_handle =
      model_load_gltf("assets/models/gltf/DamagedHelmet/glTF/DamagedHelmet.gltf", false);
  INFO("GLTF loaded successfully!");
  model* helmet = MODEL_GET(helmet_handle);

  vec3 camera_pos = vec3(5., 0., 0.);
  vec3 camera_front = vec3_normalise(vec3_negate(camera_pos));
  camera cam = camera_create(camera_pos, camera_front, VEC3_NEG_Z, deg_to_rad(45.0));
  // 4. create lights

  // directional (sun) light setup
  directional_light dir_light = { .direction = (vec3){ -0.2, -1.0, -0.3 },
                                  .ambient = (vec3){ 0.2, 0.2, 0.2 },
                                  .diffuse = (vec3){ 0.5, 0.5, 0.5 },
                                  .specular = (vec3){ 1.0, 1.0, 1.0 } };
  // point lights setup
  for (int i = 0; i < 4; i++) {
    point_lights[i].position = pointlight_positions[i];
    point_lights[i].ambient = (vec3){ 0.05, 0.05, 0.05 };
    point_lights[i].diffuse = (vec3){ 0.8, 0.8, 0.8 };
    point_lights[i].specular = (vec3){ 1.0, 1.0, 1.0 };
    point_lights[i].constant = 1.0;
    point_lights[i].linear = 0.09;
    point_lights[i].quadratic = 0.032;
  }

  scene our_scene;
  scene_init(&our_scene);
  memcpy(&our_scene.point_lights, &point_lights, sizeof(point_light[4]));
  our_scene.point_lights_count = 4;
  our_scene.dir_light = dir_light;

  while (!should_exit(&g_core)) {
    input_update(&g_core.input);

    currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    render_frame_begin(&g_core.renderer);

    // Draw the model
    static f32 angle = 0.0, rot_speed = 0.5;
    quat rot = quat_from_axis_angle(VEC3_Z, fmod(angle, TAU), true);
    angle += (rot_speed * deltaTime);
    transform model_tf = transform_create(vec3(0.0, 0.1, -0.1), rot, 1.8);
    mat4 model = transform_to_mat(&model_tf);

    draw_mesh(&helmet->meshes->data[0], &model, &cam);

    render_frame_end(&g_core.renderer);
  }

  renderer_shutdown(&g_core.renderer);

  return 0;
}
