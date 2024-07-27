//! Celeritas bindings wrapper library

#![warn(missing_docs)]
#![cfg_attr(docsrs, feature(doc_cfg))]

pub mod prelude;

use std::{
    fs::{self, File},
    io::Write,
    path::Path,
};

pub use celeritas_sys as ffi;
use celeritas_sys::{DirectionalLight, PointLight, Vec3};
use serde::{Deserialize, Serialize};

/// Wrapper around a string that is the path to a gltf model **relative** to the configured
/// `ASSETS` folder
#[derive(Debug, Serialize, Deserialize)]
pub struct ModelPath(String);

/// Scene that can be saved and loaded from disk
#[derive(Debug, Serialize, Deserialize)]
pub struct SerializableScene {
    pub sun: DirectionalLight,
    pub point_lights: [Option<PointLight>; 4],
    pub camera_orientation: (Vec3, Vec3),
    pub models: Vec<ModelPath>,
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
