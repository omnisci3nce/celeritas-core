// Load a model
// Animate it
// Collide with the ground
// Have a skybox

#include <assert.h>
#include "camera.h"
#include "core.h"
#include "input.h"
#include "keys.h"
#include "loaders.h"
#include "maths.h"
#include "primitives.h"
#include "ral_types.h"
#include "render.h"
#include "render_scene.h"
#include "render_types.h"
#include "shadows.h"
#include "skybox.h"
#include "terrain.h"

static const char* faces[6] = { "assets/demo/skybox/right.jpg", "assets/demo/skybox/left.jpg",
                                "assets/demo/skybox/top.jpg",   "assets/demo/skybox/bottom.jpg",
                                "assets/demo/skybox/front.jpg", "assets/demo/skybox/back.jpg" };

int main() {
  Core_Bringup(NULL);

  // TODO: Load humanoid model + weapon
  // TODO: Animate it with WASD keys
  // TODO: Skybox (ALMOST)
  // TODO: Add a ground terrain
  // TODO: Move camera with model

  // --- Render Scene
  Vec3 camera_pos = vec3(0.0, 5.0, 0.0);
  Camera cam = Camera_Create(camera_pos, VEC3_NEG_Z, VEC3_Y, 45.0);
  SetCamera(cam);  // update the camera in RenderScene

  DirectionalLight sun = {
    .ambient = vec3(1.0, 1.0, 1.0),
  };
  SetMainLight(sun);

  // --- Terrain
  Heightmap hmap = Heightmap_FromImage(str8("assets/test_heightmap.png"));
  Terrain_Storage* terrain = Render_GetTerrainStorage();
  Terrain_LoadHeightmap(terrain, hmap, 2.0, false);
  // assert(Terrain_IsActive());

  // --- Skybox
  Skybox skybox = Skybox_Create(faces, 6);

  // --- Models
  // ModelHandle player_model = ModelLoad_gltf("Player Model", "assets/demo/player.gltf");
  // ModelHandle sword_model = ModelLoad("Sword Model", "assets/demo/sword.gltf");

  // create a wooden crate - loads mesh and material directly rather than via loading a model from a
  // gltf file
  Geometry cube_geo = Geo_CreateCuboid(f32x3(2.0, 2.0, 2.0));
  Mesh crate_mesh = Mesh_Create(&cube_geo, false);  // dont free as we may use later
  TextureHandle albedo_map = TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_basecolor.jpg");
  TextureHandle roughness_map =
      TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_roughness.jpg");
  TextureHandle normal_map = TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_normal.jpg");
  TextureHandle ao_map =
      TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_ambientOcclusion.jpg");
  Material crate_mat = { .name = "Wood_Crate",
                         .kind = MAT_PBR,
                         .metal_roughness_combined = true,
                         .pbr_albedo_map = albedo_map,
                         .pbr_metallic_map = roughness_map,
                         .pbr_normal_map = normal_map,
                         .pbr_ao_map = ao_map };

  // ModelHandle cube_handle = ModelLoad_gltf("assets/models/gltf/Cube/glTF/Cube.gltf", false);
  // ModelHandle cube_handle = ModelLoad_gltf("../../assets/prototyper/prototyper_m.gltf", false);
  // Model* Cube = MODEL_GET(cube_handle);
  // RenderEnt cube_r = { .mesh = &Cube->meshes->data[0],
  //                      .material = &Cube->materials->data[0],
  //                      .affine = mat4_ident(),
  //                      .casts_shadows = true };

  RenderEnt crate_renderable = {
    .mesh = &crate_mesh, .material = &crate_mat, .affine = mat4_scale(3.0), .casts_shadows = true
  };

  RenderEnt entities[] = { crate_renderable };
  size_t entity_count = 1;

  // --- Transforms
  // TransformHierarchy* scene_tree =  TransformHierarchy_Create();
  // TODO: parent camera to model - to start with I can just manually update it every frame
  // TODO: query joints of the gltf to get the hand bone to parent a sword to

  bool draw_debug = true;

  while (!ShouldExit()) {
    Frame_Begin();
    if (key_just_released(KEYCODE_TAB)) {
      draw_debug = !draw_debug;
    }

    Camera_Update(&cam);
    SetCamera(cam);

    // BEGIN Draw calls

    // Shadow_Run(entities, entity_count);
    printf("cam pos: %f %f %f cam frontL %f %f %f\n", cam.position.x, cam.position.y,
           cam.position.z, cam.front.x, cam.front.y, cam.front.z);

    if (draw_debug) {
      // draw the player model with shadows
      Render_RenderEntities(entities, entity_count);
      // Render_DrawTerrain();
      Skybox_Draw(&skybox, cam);
    } else {
      Shadow_DrawDebugQuad();
    }

    // Terrain_Draw(terrain);

    // END Draw calls
    Frame_Draw();
    Frame_End();
  }

  Core_Shutdown();
  return 0;
}
