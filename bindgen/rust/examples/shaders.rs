use celeritas::ral::{ShaderData, ShaderDataLayout};
use celeritas_sys::{
    Mat4, ShaderVisibility_VISIBILITY_COMPUTE, ShaderVisibility_VISIBILITY_FRAGMENT,
    ShaderVisibility_VISIBILITY_VERTEX,
};

struct MVP {
    model: Mat4,
    view: Mat4,
    proj: Mat4,
}

fn shader_vis_all() -> u32 {
    ShaderVisibility_VISIBILITY_VERTEX
        | ShaderVisibility_VISIBILITY_FRAGMENT
        | ShaderVisibility_VISIBILITY_COMPUTE
}

impl ShaderData for MVP {
    fn layout() -> ShaderDataLayout {
        let mut bindings = [ShaderBinding::default(); 8];
        // bindings[0] = ShaderBinding {
        //     label: unsafe { CStr::from_bytes_with_nul_unchecked(b"MVP\0").as_ptr() },
        //     kind: ShaderBindingKind_BINDING_BYTES,
        //     vis: shader_vis_all(),
        //     data: ShaderBinding__bindgen_ty_1 {
        //         bytes: ShaderBinding__bindgen_ty_1__bindgen_ty_1 {
        //             size: std::mem::size_of::<MVP>() as u32,
        //             data: ptr::null_mut(),
        //         },
        //     },
        // };
        ShaderDataLayout {
            bindings: todo!(),
            binding_count: 1,
        }
    }

    fn bind(&self) {
        let layout = Self::layout();
        for i in 0..layout.binding_count {
            let binding = &layout.bindings[i];
            // match binding.kind {}
        }
    }
}

fn main() {}
