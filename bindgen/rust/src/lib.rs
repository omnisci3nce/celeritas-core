//! Celeritas bindings wrapper library

#![warn(missing_docs)]
#![cfg_attr(docsrs, feature(doc_cfg))]

pub use celeritas_sys as ffi;

/// Commonly used types
pub mod prelude;

pub mod ral;
pub mod resources;
pub mod shader;

use std::{
    ffi::CString,
    fs::{self, File},
    io::Write,
    path::Path,
};

use celeritas_sys::{
    Camera, Camera_Create, Core_Bringup, Core_Shutdown, DirectionalLight, PointLight, RenderEnt,
    Transform, Vec3,
};
use serde::{Deserialize, Serialize};

/// Wrapper around a string that is the path to a gltf model **relative** to the configured
/// `ASSETS` folder
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ModelPath(pub String);

#[deprecated]
#[derive(Debug, Serialize, Deserialize)]
pub struct ModelNode {
    pub model_path: ModelPath,
    pub transform: Transform,
}

/// Scene that can be saved and loaded from disk
#[derive(Debug, Serialize, Deserialize)]
pub struct SerializableScene {
    /// main light
    pub sun: DirectionalLight,
    pub point_lights: [Option<PointLight>; 4],
    pub camera_orientation: (Vec3, Vec3),
    pub models: Vec<ModelNode>,
}

// Runtime Scene <-> Serialized Scene

impl SerializableScene {
    /// TODO: docs
    pub fn store_to_file(&self, filepath: &Path) {
        let mut file = File::create(filepath).expect("creation failed");
        let json = serde_json::to_string(&self).expect("serialize failed");
        file.write(json.as_bytes()).expect("writing failed");
    }
    /// TODO: docs
    pub fn load_from_file(filepath: &Path) -> Self {
        let contents = fs::read_to_string(filepath).expect("Filepath should be open and read-able");

        serde_json::from_str(&contents).expect("Should be deserializable")
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Light {
    Point(ffi::PointLight),
    Directional(ffi::DirectionalLight),
    // Spot(ffi::Spotlight)
}

pub struct Core {
    _window_name: CString,
}
impl Core {
    pub fn init(window_name: &str, window: Option<*mut ffi::GLFWwindow>) -> Self {
        let name = CString::new(window_name).unwrap();
        let window_ptr = window.unwrap_or(std::ptr::null_mut());
        unsafe { Core_Bringup(name.as_ptr() as *const _, window_ptr) };
        Self { _window_name: name }
    }
}

impl Drop for Core {
    fn drop(&mut self) {
        unsafe { Core_Shutdown() }
    }
}

// pub struct Renderable {
//     pub mesh: MeshHandle,
//     pub material: MaterialHandle,
//     pub affine: Mat4,
//     pub bounding_box: Bbox_3D,
//     pub flags: RenderEntityFlags,
// }

bitflags::bitflags! {
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    pub struct RenderableFlags : u32 {

    }
}
