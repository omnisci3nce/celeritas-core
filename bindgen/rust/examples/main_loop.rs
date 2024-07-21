use celeritas::*;

unsafe fn run_game() {
  // init
  Core_Bringup();

  // let mut cube_geo = Geo_CreateCuboid(Vec3 { x: 1.0, y: 1.0, z: 1.0 });
  // let cube = Mesh_Create(&mut cube_geo, false);

  let camera_pos = Vec3 { x: 0.0, y: 2.0, z: -3.0 };
  let pos_y = Vec3 { x: 0., y: 1.0, z: 0. };
  let camera = Camera_Create(camera_pos, vec3_normalise(vec3_negate(camera_pos)), pos_y, 45.0);
  SetCamera(camera);
  // let camera = Camera_Create(camera_pos, vec3_normalise(vec3_negate(camera_pos)), VEC3_Y, 45.0);
  // SetCamera(cam);  // update the camera in RenderScene

  let whatever = Vec3 { x: 1.0, y: 1.0, z: 1.0 };
  let sun = DirectionalLight { direction: whatever, ambient: whatever, diffuse: whatever, specular: whatever };
  SetMainLight(sun);

  // Skybox skybox = Skybox_Create(faces, 6);
  let skybox = Skybox_Create(face_paths, 6);

    while !ShouldExit() {
      Frame_Begin();



      Frame_End();
    }
}

fn main() {
    println!("Running from Rust!");

    unsafe {
        run_game();
    }
}
