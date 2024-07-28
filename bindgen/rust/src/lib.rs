//! Celeritas bindings wrapper library

#![warn(missing_docs)]
#![cfg_attr(docsrs, feature(doc_cfg))]

pub use celeritas_sys as ffi;

/// Commonly used types
pub mod prelude;

use std::{
    fs::{self, File},
    io::Write,
    path::Path,
};

use celeritas_sys::{DirectionalLight, PointLight, Transform, Vec3};
use serde::{Deserialize, Serialize};

/// Wrapper around a string that is the path to a gltf model **relative** to the configured
/// `ASSETS` folder
#[derive(Debug, Serialize, Deserialize)]
pub struct ModelPath(String);

///
#[derive(Debug, Serialize, Deserialize)]
pub struct ModelNode {
    model_path: ModelPath,
    transform: Transform
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

#[derive(Debug, Clone)]
pub enum Light {
    Point(ffi::PointLight),
    Directional(ffi::DirectionalLight),
    // Spot(ffi::Spotlight)
}