// Load a model
// Animate it
// Collide with the ground
// Have a skybox

#include <assert.h>
#include "camera.h"
#include "core.h"
#include "maths.h"
#include "render.h"
#include "render_scene.h"
#include "render_types.h"
#include "skybox.h"
#include "str.h"
#include "terrain.h"
#include "transform_hierarchy.h"

static const char* faces[6] = { "assets/demo/skybox/left.jpg",  "assets/demo/skybox/right.jpg",
                                "assets/demo/skybox/front.jpg", "assets/demo/skybox/back.jpg",
                                "assets/demo/skybox/top.jpg",   "assets/demo/skybox/bottom.jpg" };

int main() {
  Core_Bringup();

  Vec3 camera_pos = vec3(0.0, 4.0, 8.0);
  Camera cam = Camera_Create(camera_pos, vec3_negate(camera_pos), VEC3_Y, 45.0);
  SetCamera(cam);  // update the camera in RenderScene

  // TODO: Load humanoid model + weapon
  // TODO: Animate it with WASD keys
  // TODO: Skybox
  // TODO: Add a ground terrain
  // TODO: Move camera with model

  // --- Terrain
  Heightmap terrain = Heightmap_FromImage(str8("assets/demo/heightmap.png"));
  Terrain_LoadHeightmap(terrain, true);
  assert(Terrain_IsActive());

  // --- Skybox
  Skybox skybox = Skybox_Create(faces, 6);

  // --- Models
  ModelHandle player_model = ModelLoad("Player Model", "assets/demo/player.gltf");
  ModelHandle sword_model = ModelLoad("Sword Model", "assets/demo/sword.gltf");

  // --- Transforms
  // TransformHierarchy* scene_tree =  TransformHierarchy_Create();
  // TODO: parent camera to model - to start with I can just manually update it every frame
  // TODO: query joints of the gltf to get the hand bone to parent a sword to

  RenderEnt player_r = { .model = player_model, .affine = mat4_ident(), .casts_shadows = true };
  while (!ShouldExit()) {
    Frame_Begin();

    // BEGIN Draw calls

    // draw the player model with shadows

    Render_DrawTerrain();
    Skybox_Draw(&skybox);

    // END Draw calls
    Frame_Draw();
    Frame_End();
  }

  Core_Shutdown();
  return 0;
}
