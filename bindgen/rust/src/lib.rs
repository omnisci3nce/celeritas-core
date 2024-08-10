//! Celeritas bindings wrapper library

#![warn(missing_docs)]
#![cfg_attr(docsrs, feature(doc_cfg))]

pub use celeritas_sys as ffi;
use glam::Vec3;

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
    ptr::addr_of_mut,
};

use celeritas_sys::{
    Core_Bringup, Core_Shutdown, DirectionalLight, Material, Material_Insert, Model, PointLight,
    TextureHandle, Transform,
};
use serde::{Deserialize, Serialize};

pub trait IntoFFI {
    type FFIType;
    unsafe fn into_ffi(self) -> Self::FFIType;
}

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

type M = Material;
/*
pub name: [::std::os::raw::c_char; 64usize],
pub kind: MaterialKind,
pub base_colour: Vec3,
pub metallic: f32_,
pub roughness: f32_,
pub ambient_occlusion: f32_,
pub albedo_map: TextureHandle,
pub normal_map: TextureHandle,
pub metallic_roughness_map: TextureHandle,
pub ambient_occlusion_map: TextureHandle, */

#[derive(Debug, Clone, Default, PartialEq)]
pub struct PBRMaterial {
    pub name: String,
    pub base_colour: glam::Vec3,
    pub metallic: f32,
    pub roughness: f32,
    pub ambient_occlusion: f32,
    pub albedo_map: Option<TextureHandle>,
    pub normal_map: Option<TextureHandle>,
    pub metallic_roughness_map: Option<TextureHandle>,
    pub ambient_occlusion_map: Option<TextureHandle>,
}
impl PBRMaterial {
    /// Creates the  material in the C core returning a handle to it
    pub fn create(mat: Self) -> ffi::MaterialHandle {
        let mut ffi_mat = ffi::Material::from(mat);
        unsafe { Material_Insert(addr_of_mut!(ffi_mat)) }
    }
}

impl From<PBRMaterial> for ffi::Material {
    fn from(value: PBRMaterial) -> Self {
        todo!("impl conv for materials")
    }
}
