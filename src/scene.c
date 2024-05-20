#include "scene.h"
#include "core.h"
#include "log.h"
#include "maths.h"
#include "render_types.h"

extern core g_core;

void scene_init(scene *s) {
  memset(s, 0, sizeof(scene));
  s->renderables = render_entity_darray_new(10);
  // default camera position - moved slightly along Z axis looking at 0,0,0
  vec3 cam_pos = vec3_create(0, 0, -5);
  s->camera = camera_create(cam_pos, vec3_negate(cam_pos), VEC3_Y, deg_to_rad(45.0));
}
void scene_free(scene *s) { render_entity_darray_free(s->renderables); }

void scene_set_dir_light(directional_light light) { g_core.default_scene.dir_light = light; }
void scene_add_point_light(point_light light) {
  scene s = g_core.default_scene;
  if (s.point_lights_count == 4) {
    WARN("Already 4 point lights, we can't add more.");
  } else {
    s.point_lights[s.point_lights_count] = light;
    s.point_lights_count++;
  }
}
void scene_add_model(model_handle model, transform3d transform) {
  render_entity renderable = { .model = model, .tf = transform };
  render_entity_darray_push(g_core.default_scene.renderables, renderable);
}

bool scene_remove_model(model_handle model) {
  scene s = g_core.default_scene;
  for (u32 i = 0; i <= s.renderables->len; i++) {
    if (s.renderables->data[i].model.raw == model.raw) {
      // TODO: add remove function to darray
    }
  }
  return true;
}

void scene_set_model_transform(model_handle model, transform3d new_transform) {
  scene s = g_core.default_scene;
  for (u32 i = 0; i <= s.renderables->len; i++) {
    if (s.renderables->data[i].model.raw == model.raw) {
      s.renderables->data[i].tf = new_transform;
    }
  }
}

void scene_set_camera(vec3 pos, vec3 front) {
  scene s = g_core.default_scene;
  s.camera.position = pos;
  s.camera.front = front;
}
