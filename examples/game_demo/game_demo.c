// Load a model
// Animate it
// Collide with the ground
// Have a skybox

#include <assert.h>
#include "camera.h"
#include "core.h"
#include "grid.h"
#include "immdraw.h"
#include "input.h"
#include "keys.h"
#include "loaders.h"
#include "maths.h"
#include "maths_types.h"
#include "pbr.h"
#include "primitives.h"
#include "ral_types.h"
#include "render.h"
#include "render_types.h"
#include "shadows.h"
#include "skybox.h"
#include "terrain.h"

static const char* faces[6] = { "assets/skybox/right.jpg", "assets/skybox/left.jpg",
                                "assets/skybox/top.jpg",   "assets/skybox/bottom.jpg",
                                "assets/skybox/front.jpg", "assets/skybox/back.jpg" };

int main() {
  Core_Bringup("Celeritas: Game Demo", NULL);

  // TODO: Load humanoid model + weapon
  // TODO: Animate it with WASD keys
  // TODO: Skybox (ALMOST)
  // TODO: Add a ground terrain
  // TODO: Move camera with model

  // --- Render Scene
  Vec3 camera_pos = vec3(0.0, 1.0, 2.0);
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

  // --- Skybox
  Skybox skybox = Skybox_Create(faces, 6);

  // --- Models
  // create a wooden crate - loads mesh and material directly rather than via loading a model from a
  // gltf file
  Geometry cube_geo = Geo_CreateCuboid(f32x3(2.0, 2.0, 2.0));
  Mesh crate_mesh = Mesh_Create(&cube_geo, false);  // dont free as we may use later
  MeshHandle crate_mesh_handle = Mesh_pool_insert(Render_GetMeshPool(), &crate_mesh);
  // TextureHandle albedo_map =
  // TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_basecolor.jpg");
  TextureHandle roughness_map =
      TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_roughness.jpg");
  TextureHandle normal_map = TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_normal.jpg");
  TextureHandle ao_map =
      TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_ambientOcclusion.jpg");
  Material crate_mat = PBRMaterialDefault();
  // crate_mat.name = "Wood_Crate";
  //  crate_mat.albedo_map = albedo_map;
  crate_mat.metallic_roughness_map = roughness_map;
  crate_mat.normal_map = normal_map;
  crate_mat.ambient_occlusion_map = ao_map;
  crate_mat.base_colour = vec3(1.0, 1.0, 1.0);
  crate_mat.metallic = 0.0;
  MaterialHandle crate_mat_handle = Material_pool_insert(Render_GetMaterialPool(), &crate_mat);
  // ModelHandle cube_handle = ModelLoad_gltf("assets/models/gltf/Cube/glTF/Cube.gltf", false);
  ModelHandle cube_handle =
      // ModelLoad_gltf("assets/models/gltf/DamagedHelmet/glTF/DamagedHelmet.gltf", false);
      ModelLoad_gltf("../../assets/prototyper/prototyper_m.gltf", false);

  RenderEnt_darray* render_entities = RenderEnt_darray_new(1);

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

    FlyCamera_Update(&cam);
    SetCamera(cam);

    // BEGIN Draw calls
    RenderEnt_darray_clear(render_entities);  // we re-extract every frame
    // Quat rot = quat_from_axis_angle(VEC3_X, -HALF_PI, true);
    // Mat4 affine = mat4_rotation(rot);
    Mat4 affine = mat4_ident();
    ModelExtractRenderEnts(render_entities, cube_handle, affine, REND_ENT_CASTS_SHADOWS);

    // Shadow_Run(entities, entity_count);
    // Quat rot = quat_from_axis_angle(VEC3_X, HALF_PI, true);
    Immdraw_Sphere(transform_create(VEC3_ZERO, quat_ident(), 0.5), vec4(1.0, 0.0, 0.0, 1.0), false);
    Immdraw_Cuboid(transform_create(vec3(2.0, 0.0, 0.0), quat_ident(), 1.0),
                   vec4(1.0, 0.0, 0.0, 1.0), false);

    // if (draw_debug) {
    //   // draw the player model with shadows
    //   Render_RenderEntities(render_entities->data, render_entities->len);
    //   // Render_DrawTerrain();
    //   Skybox_Draw(&skybox, cam);
    // } else {
    //   Shadow_DrawDebugQuad();
    // }

    // Terrain_Draw(terrain);
    // Grid_Draw();

    // END Draw calls
    Frame_Draw();
    Frame_End();
  }

  Core_Shutdown();
  return 0;
}
