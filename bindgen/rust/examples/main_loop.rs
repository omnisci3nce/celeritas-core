use std::{ffi::{ CString}, ptr::{self, addr_of_mut}};

use celeritas::*;

unsafe fn run_game() {
    // init
    let p: *mut GLFWwindow = ptr::null_mut();
    Core_Bringup(p);

    let core = get_global_core();
    let glfw_window_ptr = Core_GetGlfwWindowPtr(core);

    let camera_pos = Vec3 {
        x: 0.0,
        y: 2.0,
        z: -3.0,
    };
    let camera = Camera_Create(
        camera_pos,
        vec3_normalise(vec3_negate(camera_pos)),
        VEC3_Y,
        45.0,
    );
    SetCamera(camera);

    let whatever = Vec3 {
        x: 1.0,
        y: 1.0,
        z: 1.0,
    };
    let sun = DirectionalLight {
        direction: whatever,
        ambient: whatever,
        diffuse: whatever,
        specular: whatever,
    };
    SetMainLight(sun);

//     Geometry cube_geo = Geo_CreateCuboid(f32x3(2.0, 2.0, 2.0));
//   Mesh crate_mesh = Mesh_Create(&cube_geo, false);  // dont free as we may use later
//   TextureHandle albedo_map = TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_basecolor.jpg");
//   TextureHandle roughness_map =
//       TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_roughness.jpg");
//   TextureHandle normal_map = TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_normal.jpg");
//   TextureHandle ao_map =
//       TextureLoadFromFile("assets/demo/crate/Wood_Crate_001_ambientOcclusion.jpg");
//   Material crate_mat = { .name = "Wood_Crate",
//                          .kind = MAT_PBR,
//                          .metal_roughness_combined = true,
//                          .pbr_albedo_map = albedo_map,
//                          .pbr_metallic_map = roughness_map,
//                          .pbr_normal_map = normal_map,
//                          .pbr_ao_map = ao_map };
    let mut cube_geo = Geo_CreateCuboid(f32x3 { x: 2.0, y: 2.0, z: 2.0 });
    let mut crate_mesh = Mesh_Create(addr_of_mut!(cube_geo), false);
    let albedo_map = TextureLoadFromFile(CString::new("assets/demo/crate/Wood_Crate_001_basecolor.jpg").unwrap().as_ptr() as *const i8);
    let roughness_map = TextureLoadFromFile(CString::new("assets/demo/crate/Wood_Crate_001_roughness.jpg").unwrap().as_ptr() as *const i8);
    let normal_map = TextureLoadFromFile(CString::new("assets/demo/crate/Wood_Crate_001_normal.jpg").unwrap().as_ptr() as *const i8);
    let ao_map = TextureLoadFromFile(CString::new("assets/demo/crate/Wood_Crate_001_ambientOcclusion.jpg").unwrap().as_ptr() as *const i8);
    let name: [i8; 64] = [0; 64];
    let mut crate_mat = Material {
        name: name,
        kind: 0,
        param_albedo: VEC3_ZERO,
        param_metallic: 0.0,
        param_roughness: 0.0,
        param_ao: 0.0,
        pbr_albedo_map: albedo_map,
        pbr_normal_map: normal_map,
        metal_roughness_combined: true,
        pbr_metallic_map: TextureHandle { raw: 99999 },
        pbr_roughness_map: roughness_map,
        pbr_ao_map: ao_map
    };
    let crate_renderent = RenderEnt {
        mesh: addr_of_mut!(crate_mesh),
        material: addr_of_mut!(crate_mat),
        affine: mat4_ident(),
        casts_shadows: true,
    };
    let mut render_entities: [RenderEnt; 1] = [crate_renderent]; 

    // RenderEnt crate_renderable = {
    //     .mesh = &crate_mesh, .material = &crate_mat, .affine = mat4_scale(3.0), .casts_shadows = true
    //   };
    
    //   RenderEnt entities[] = { cube_r, crate_renderable };
    //   size_t entity_count = 1;

    // main loop
    while !ShouldExit() {
        Frame_Begin();

        Render_RenderEntities(render_entities.as_mut_ptr(), render_entities.len());

        Frame_End();
    }
}

fn main() {
    println!("Running from Rust!");

    unsafe {
        run_game();
    }
}
