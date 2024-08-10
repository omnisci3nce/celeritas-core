use std::ffi::CString;

use celeritas_sys::{ModelHandle, ModelLoad_gltf};

/// Load a gltf from disk
pub fn model_load_gltf(path: &str) -> Option<ModelHandle> {
    let path_str = CString::new(path).unwrap();
    let handle = unsafe { ModelLoad_gltf(path_str.as_ptr() as *const _, false) };
    Some(handle)
}
