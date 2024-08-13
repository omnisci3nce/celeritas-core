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
  double deltaTime;

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

  Vec3 cam_pos = vec3_create(0, 1.2, 4);
  Camera cam = Camera_Create(cam_pos, VEC3_NEG_Z, VEC3_Y, deg_to_rad(45.0));
  SetCamera(cam);

  // Main loop
  const f32 camera_lateral_speed = 0.2;
  const f32 camera_zoom_speed = 0.10;

  // animation
  // animation_clip track = simple_skin->animations->data[0];
  // f64 total_time = 0.0;

  while (!ShouldExit()) {
    Frame_Begin();

    currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    // total_time += deltaTime;
    // printf("delta time %f\n", deltaTime);
    // f64 t = fmod(total_time, track.rotation->max);
    // INFO("Total time: %f", t);

    // bone rotation
    // Quat rot = animation_sample(track.rotation, t).rotation;

    // m->bones->data[1].transform_components.rotation = rot;

    // quat rot = quat_ident();
    // Transform tf = transform_create(VEC3_ZERO, quat_ident(), 1.0);

    // TODO: Drawing should still just use the PBR pipeline
    Mesh* m = Mesh_Get(simple_skin->meshes[0]);
    RenderEnt render_ents[1] = {
        (RenderEnt ){ .mesh = simple_skin->meshes[0],
                           .material = m->material,
                           .animation_clip = simple_skin->animations->data[0],
                           .is_playing = true,
                           .t = 0.0,
                           .affine = mat4_ident(),
                           .flags = 0 }
    };
    // ModelExtractRenderEnts(rend_ents, handle, mat4_translation(vec3(0, 0, 0)), 0);

    // draw_skinned_model(&core->renderer, &game.camera, simple_skin, tf, &our_scene);
    Render_RenderEntities(render_ents, 1);

    // Animation_VisualiseJoints(&m->armature);

    RenderEnt_darray_clear(rend_ents);

    Frame_End();
  }

  INFO("Shutting down");

  return 0;
}
