use std::{
    ffi::CString,
    fs::{self, File},
    io::Write,
    path::Path,
    ptr::{self, addr_of_mut},
};


use egui_backend::egui::{vec2, Pos2, Rect};
use egui_glfw as egui_backend;
use egui_glfw::glfw::{fail_on_errors, Context};

use egui_glfw::glfw;

use celeritas::*;
use ffi::*;

fn main() {
    unsafe {
        let p: *mut GLFWwindow = ptr::null_mut();
        Core_Bringup(p);

        // let core = get_global_core();

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
        let mut camera = Camera_Create(
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

        let mut cube_geo = Geo_CreateCuboid(f32x3 {
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

    }
}
