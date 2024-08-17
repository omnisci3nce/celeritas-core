#include <assert.h>
#include <glfw3.h>

#include "animation.h"
#include "camera.h"
#include "core.h"
#include "loaders.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "pbr.h"
#include "render.h"
#include "render_types.h"

int main() {
  double currentFrame = glfwGetTime();
  double lastFrame = currentFrame;
  double delta_time;

  Core_Bringup("Skinned Animation", NULL);

  ModelHandle handle = ModelLoad_gltf("assets/models/gltf/SimpleSkin/glTF/SimpleSkin.gltf", false);
  Model* simple_skin = MODEL_GET(handle);

  RenderEnt_darray* rend_ents = RenderEnt_darray_new(1);

  // Okay, right here we've loaded the model. let's assert some facts
  // assert(simple_skin->animations->len == 1);
  // assert(simple_skin->animations->data[0].rotation != NULL);
  // assert(simple_skin->animations->data[0].translation == NULL);
  // assert(simple_skin->animations->data[0].scale == NULL);

  // mesh* m = &simple_skin->meshes->data[0];
  // assert(m->is_skinned);
  // assert(m->bones->len == 2);  // 1 root and 1 extra joint

  // assert(false);

  // model_upload_meshes(&core->renderer, simple_skin);

  // scene our_scene = make_default_scene();

  Vec3 cam_pos = vec3_create(1.5, 2.2, 8);
  Camera cam = Camera_Create(cam_pos, VEC3_NEG_Z, VEC3_Y, deg_to_rad(45.0));
  SetCamera(cam);

  // Main loop
  const f32 camera_lateral_speed = 0.2;
  const f32 camera_zoom_speed = 0.10;

  // animation
  // animation_clip track = simple_skin->animations->data[0];
  CASSERT(AnimationClip_darray_len(simple_skin->animations) > 0);
  AnimationClip track = simple_skin->animations->data[0];
  f64 total_time = 0.0;

  while (!ShouldExit()) {
    Frame_Begin();

    currentFrame = glfwGetTime();
    delta_time = currentFrame - lastFrame;
    lastFrame = currentFrame;
    total_time += delta_time;
    // printf("delta time %f\n", deltaTime);
    f64 t = fmod(total_time, track.channels->data[0].max);
    INFO("Delta time %f   Animation time: %f", delta_time, t);

    // bone rotation
    // Quat rot = animation_sample(track.rotation, t).rotation;

    // m->bones->data[1].transform_components.rotation = rot;

    // quat rot = quat_ident();
    // Transform tf = transform_create(VEC3_ZERO, quat_ident(), 1.0);

    // TODO: Drawing should still just use the PBR pipeline
    Mesh* m = Mesh_Get(simple_skin->meshes[0]);
    RenderEnt render_ents[1] = { (RenderEnt){ .mesh = simple_skin->meshes[0],
                                              .material = m->material,
                                              .armature = &m->armature,
                                              .affine = mat4_ident(),
                                              .flags = 0 } };
    // ModelExtractRenderEnts(rend_ents, handle, mat4_translation(vec3(0, 0, 0)), 0);

    // draw_skinned_model(&core->renderer, &game.camera, simple_skin, tf, &our_scene);
    Animation_Tick(&track, &m->armature, t);

    Render_RenderEntities(render_ents, 1);

    Animation_VisualiseJoints(&m->armature);

    RenderEnt_darray_clear(rend_ents);

    Frame_End();
  }

  INFO("Shutting down");

  return 0;
}
