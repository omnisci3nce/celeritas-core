use celeritas::ral::{
    ShaderBinding, ShaderBindingKind, ShaderData, ShaderDataLayout, ShaderVisibility,
};
use celeritas_sys::{
    GPU_EncodeBindShaderData, GPU_GetDefaultEncoder, Mat4, ShaderVisibility_VISIBILITY_COMPUTE,
    ShaderVisibility_VISIBILITY_FRAGMENT, ShaderVisibility_VISIBILITY_VERTEX,
};

#[allow(clippy::upper_case_acronyms)]
#[repr(C)]
struct MVP {
    model: Mat4,
    view: Mat4,
    proj: Mat4,
}

pub fn shader_vis_all() -> u32 {
    ShaderVisibility_VISIBILITY_VERTEX
        | ShaderVisibility_VISIBILITY_FRAGMENT
        | ShaderVisibility_VISIBILITY_COMPUTE
}

impl ShaderData for MVP {
    fn layout() -> ShaderDataLayout {
        let mut bindings: heapless::Vec<ShaderBinding, 8> = heapless::Vec::new();
        let _ = bindings.push(ShaderBinding {
            label: "MVP".to_string(),
            kind: ShaderBindingKind::Bytes {
                size: std::mem::size_of::<MVP>(),
                data: None,
            },
            vis: ShaderVisibility::all(),
        });
        ShaderDataLayout { bindings }
    }

    fn bind(&self) {
        let mut layout = Self::layout();
        let b0 = &mut layout.bindings[0];
        b0.kind = ShaderBindingKind::Bytes {
            size: std::mem::size_of::<MVP>(),
            data: Some((self as *const MVP) as *mut u8),
        };

        unsafe {
            GPU_EncodeBindShaderData(GPU_GetDefaultEncoder(), 0, layout.into_ffi_type());
        }
    }
}

fn main() {}
