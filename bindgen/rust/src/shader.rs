use std::{ffi::c_void, path::Path};

use crate::ral::{Pipeline, ShaderBinding};

pub struct Shader {
    pipeline: Pipeline,
    binding_layouts: Vec<ShaderBinding>,
}

#[no_mangle]
pub unsafe extern "C" fn rust_function(data: *mut c_void) -> celeritas_sys::ShaderDataLayout {
    todo!()
}

impl Shader {
    pub fn new(name: String, vs_path: &Path, fs_path: &Path) -> Self {
        todo!()
    }
    pub fn add_layout(&mut self) -> &mut Self {
        // TODO
        self
    }
}
