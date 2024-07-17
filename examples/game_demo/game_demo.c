// Load a model
// Animate it
// Collide with the ground
// Have a skybox

#include <assert.h>
#include "camera.h"
#include "core.h"
#include "loaders.h"
#include "maths.h"
#include "primitives.h"
#include "ral_types.h"
#include "render.h"
#include "render_scene.h"
#include "render_types.h"
#include "skybox.h"
#include "str.h"
#include "terrain.h"
#include "transform_hierarchy.h"

static const char* faces[6] = { "assets/demo/skybox/right.jpg", "assets/demo/skybox/left.jpg",
                                "assets/demo/skybox/top.jpg",   "assets/demo/skybox/bottom.jpg",
                                "assets/demo/skybox/back.jpg",  "assets/demo/skybox/front.jpg" };

int main() {
  Core_Bringup();

  // TODO: Load humanoid model + weapon
  // TODO: Animate it with WASD keys
  // TODO: Skybox
  // TODO: Add a ground terrain
  // TODO: Move camera with model

  // --- Render Scene
  Vec3 camera_pos = vec3(0.0, 4.0, 8.0);
  Camera cam = Camera_Create(camera_pos, vec3_negate(camera_pos), VEC3_Y, 45.0);
  SetCamera(cam);  // update the camera in RenderScene

  DirectionalLight sun = {
    .ambient = vec3(1.0, 1.0, 1.0),
  };
  SetMainLight(sun);

  // --- Terrain
  // Heightmap terrain = Heightmap_FromImage(str8("assets/demo/heightmap.png"));
  // Terrain_LoadHeightmap(terrain, true);
  // assert(Terrain_IsActive());

  // --- Skybox
  Skybox skybox = Skybox_Create(faces, 6);

  // --- Models
  // ModelHandle player_model = ModelLoad_gltf("Player Model", "assets/demo/player.gltf");
  // ModelHandle sword_model = ModelLoad("Sword Model", "assets/demo/sword.gltf");
  // create a wooden crate
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

  RenderEnt crate_renderable = {
    .mesh = &crate_mesh, .material = &crate_mat, .affine = mat4_ident(), .casts_shadows = true
  };

  RenderEnt entities[] = { crate_renderable };
  size_t entity_count = 1;

  // --- Transforms
  // TransformHierarchy* scene_tree =  TransformHierarchy_Create();
  // TODO: parent camera to model - to start with I can just manually update it every frame
  // TODO: query joints of the gltf to get the hand bone to parent a sword to

  // RenderEnt player_r = { .model = player_model, .affine = mat4_ident(), .casts_shadows = true };

  // RenderEnt entities[] = { player_r };

  while (!ShouldExit()) {
    Frame_Begin();

    // BEGIN Draw calls

    // draw the player model with shadows
    Render_RenderEntities(entities, entity_count);
    // Render_DrawTerrain();
    // Skybox_Draw(&skybox);

    // END Draw calls
    Frame_Draw();
    Frame_End();
  }

  Core_Shutdown();
  return 0;
}
