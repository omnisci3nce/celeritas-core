use std::path::Path;

use celeritas::{ffi::*, SerializableScene};

fn main() {
    unsafe {
        let camera_pos = Vec3 {
            x: 18.9,
            y: 10.6,
            z: 11.6,
        };
        let camera_front = Vec3 {
            x: -0.6,
            y: -0.2,
            z: -0.7,
        };
        let camera = Camera_Create(
            camera_pos,
            camera_front,
            Vec3 {
                x: 0.0,
                y: 1.0,
                z: 0.0,
            },
            45.0,
        );
        SetCamera(camera);

        let cube_geo = Geo_CreateCuboid(f32x3 {
            x: 2.0,
            y: 2.0,
            z: 2.0,
        });

        let scene = SerializableScene {
            sun: DirectionalLight {
                direction: Vec3 {
                    x: 0.0,
                    y: 1.0,
                    z: 0.0,
                },
                ambient: Vec3 {
                    x: 1.0,
                    y: 1.0,
                    z: 1.0,
                },
                diffuse: Vec3 {
                    x: 1.0,
                    y: 1.0,
                    z: 1.0,
                },
                specular: Vec3 {
                    x: 0.0,
                    y: 0.0,
                    z: 0.0,
                },
            },
            point_lights: [None, None, None, None],
            camera_orientation: (camera_pos, camera_front),
            models: vec![],
        };

        let scene_path = Path::new("default_scene.json");
        scene.store_to_file(scene_path);

        let rehydrated_scene = SerializableScene::load_from_file(scene_path);
        dbg!(&rehydrated_scene);

        // let mut crate_mesh = Mesh_Create(addr_of_mut!(cube_geo), false);
        // let albedo_map = TextureLoadFromFile(
        //     CString::new("assets/demo/crate/Wood_Crate_001_basecolor.jpg")
        //         .unwrap()
        //         .as_ptr() as *const i8,
        // );
        // let roughness_map = TextureLoadFromFile(
        //     CString::new("assets/demo/crate/Wood_Crate_001_roughness.jpg")
        //         .unwrap()
        //         .as_ptr() as *const i8,
        // );
        // let normal_map = TextureLoadFromFile(
        //     CString::new("assets/demo/crate/Wood_Crate_001_normal.jpg")
        //         .unwrap()
        //         .as_ptr() as *const i8,
        // );
        // let ao_map = TextureLoadFromFile(
        //     CString::new("assets/demo/crate/Wood_Crate_001_ambientOcclusion.jpg")
        //         .unwrap()
        //         .as_ptr() as *const i8,
        // );
        // let name: [i8; 64] = [0; 64];
        // let mut crate_mat = Material {
        //     name: name,
        //     kind: 0,
        //     param_albedo: Vec3 {
        //         x: 0.0,
        //         y: 0.0,
        //         z: 0.0,
        //     },
        //     param_metallic: 0.0,
        //     param_roughness: 0.0,
        //     param_ao: 0.0,
        //     pbr_albedo_map: albedo_map,
        //     pbr_normal_map: normal_map,
        //     metal_roughness_combined: true,
        //     pbr_metallic_map: TextureHandle { raw: 99999 },
        //     pbr_roughness_map: roughness_map,
        //     pbr_ao_map: ao_map,
        // };
        // let crate_renderent = RenderEnt {
        //     mesh: addr_of_mut!(crate_mesh),
        //     material: addr_of_mut!(crate_mat),
        //     affine: mat4_ident(),
        //     casts_shadows: true,
        // };
        // let mut render_ents: [RenderEnt; 1] = [crate_renderent];
    }
}
