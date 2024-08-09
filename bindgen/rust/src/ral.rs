//! Wrapper around the RAL code in celeritas-core

use std::ffi::c_void;

use celeritas_sys::{
    BufferHandle, GPU_CmdEncoder, GPU_CmdEncoder_BeginRender, GPU_CmdEncoder_EndRender,
    GPU_EncodeBindShaderData, GPU_GetDefaultEncoder, GPU_GetDefaultRenderpass,
    GPU_GraphicsPipeline_Create, GPU_Pipeline, GraphicsPipelineDesc,
    ShaderBindingKind_BINDING_BYTES, ShaderBinding__bindgen_ty_1,
    ShaderBinding__bindgen_ty_1__bindgen_ty_1, ShaderVisibility_VISIBILITY_COMPUTE,
    ShaderVisibility_VISIBILITY_FRAGMENT, ShaderVisibility_VISIBILITY_VERTEX, TextureHandle,
    MAX_SHADER_DATA_LAYOUTS,
};
use thiserror::Error;

/// Holds a pointer to the raw `GPU_CmdEncoder`
pub struct FrameRenderEncoder(*mut GPU_CmdEncoder);

/// Holds a pointer to the raw `GPU_Renderpass`
pub struct RenderPass(*mut celeritas_sys::GPU_Renderpass);

/// Holds a pointer to the raw `GPU_Pipeline`
pub struct Pipeline(*mut celeritas_sys::GPU_Pipeline);

impl FrameRenderEncoder {
    pub fn new(renderpass: &RenderPass) -> Self {
        let enc = unsafe {
            let enc = GPU_GetDefaultEncoder();
            GPU_CmdEncoder_BeginRender(enc, renderpass.0);
            enc
        };
        FrameRenderEncoder(enc)
    }
}

impl Drop for FrameRenderEncoder {
    fn drop(&mut self) {
        unsafe {
            GPU_CmdEncoder_EndRender(self.0);
        }
    }
}

impl FrameRenderEncoder {
    pub fn set_vertex_buffer(&self, buf: BufferHandle) {
        // TODO: Get buffer ptr from handle
        // TODO: assert that buffer type is vertex
        todo!()
    }
    pub fn set_index_buffer(&self, buf: BufferHandle) {
        // TODO: Get buffer ptr from handle
        // TODO: assert that buffer type is index
        todo!()
    }
    pub fn bind<S: ShaderData>(&mut self, data: &S) {
        // TODO: fill ShaderDataLayout with correct data
        unsafe { GPU_EncodeBindShaderData(self.0, 0, todo!()) }
    }
}

pub struct PipelineBuilder {
    renderpass: Option<RenderPass>,
    data_layouts: Vec<ShaderDataLayout>,
}

#[derive(Debug, Error)]
pub enum RALError {
    #[error("exceeded maximum of 8 layouts for a pipeline")]
    TooManyShaderDataLayouts,
}

impl PipelineBuilder {
    pub fn build(self) -> Result<Pipeline, RALError> {
        let layouts = [celeritas_sys::ShaderDataLayout::default(); 8];
        if self.data_layouts.len() > MAX_SHADER_DATA_LAYOUTS as usize {
            return Err(RALError::TooManyShaderDataLayouts);
        }
        for (i, layout) in self.data_layouts.iter().enumerate().take(8) {
            // layouts[i] = celeritas_sys::ShaderDataLayout::from(layout);
        }

        let mut desc = GraphicsPipelineDesc {
            debug_name: todo!(),
            vertex_desc: todo!(),
            vs: todo!(),
            fs: todo!(),
            data_layouts: layouts,
            data_layouts_count: layouts.len() as u32,
            wireframe: false,
            depth_test: true,
        };
        let p = unsafe {
            GPU_GraphicsPipeline_Create(
                desc,
                self.renderpass
                    .map(|r| r.0)
                    .unwrap_or(GPU_GetDefaultRenderpass()),
            )
        };
        Ok(Pipeline(p))
    }

    pub fn add_shader_layout<S: ShaderData>(&mut self) -> &mut Self {
        let layout = S::layout();
        self.data_layouts.push(layout);
        self
    }
}

impl Pipeline {
    pub fn raw_ptr(&self) -> *mut GPU_Pipeline {
        self.0
    }
}

///
pub trait ShaderData {
    ///
    fn layout() -> ShaderDataLayout;
    ///
    fn bind(&self);

    // fn bind_texture(&self, binding_name: &str, handle: TextureHandle);
    // fn bind_buffer(&self, binding_name: &str, handle: BufferHandle);
}

#[derive(Clone)]
pub struct ShaderBinding {
    pub label: String,
    // pub label: *const ::std::os::raw::c_char,
    pub kind: ShaderBindingKind,
    pub vis: ShaderVisibility,
    // pub data: ShaderBinding__bindgen_ty_1,
}

#[derive(Clone)]
pub enum ShaderBindingKind {
    Bytes { size: usize, data: Option<*mut u8> },
    Buffer(Option<BufferHandle>),
    Texture(Option<TextureHandle>),
}

bitflags::bitflags! {
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    pub struct ShaderVisibility : u32 {
        const VERTEX = 1 << ShaderVisibility_VISIBILITY_VERTEX;
        const FRAGMENT = 1 << ShaderVisibility_VISIBILITY_FRAGMENT;
        const COMPUTE = 1 << ShaderVisibility_VISIBILITY_COMPUTE;
    }
}
impl Default for ShaderVisibility {
    fn default() -> Self {
        ShaderVisibility::all()
    }
}

#[derive(Default)]
pub struct ShaderDataLayout {
    pub bindings: heapless::Vec<ShaderBinding, 8>,
}
impl ShaderDataLayout {
    pub fn into_ffi_type(self) -> celeritas_sys::ShaderDataLayout {
        let mut bindings = [celeritas_sys::ShaderBinding::default(); 8];
        for (i, b) in self.bindings.iter().enumerate().take(8) {
            bindings[i] = match b.kind {
                ShaderBindingKind::Bytes { size, data } => celeritas_sys::ShaderBinding {
                    label: b.label.as_ptr() as *const i8,
                    kind: ShaderBindingKind_BINDING_BYTES,
                    vis: ShaderVisibility_VISIBILITY_VERTEX,
                    data: ShaderBinding__bindgen_ty_1 {
                        bytes: ShaderBinding__bindgen_ty_1__bindgen_ty_1 {
                            size: size as u32,
                            data: data.unwrap() as *mut c_void,
                        },
                    },
                },
                ShaderBindingKind::Buffer(_) => todo!(),
                ShaderBindingKind::Texture(_) => todo!(),
            };
        }
        celeritas_sys::ShaderDataLayout {
            bindings,
            binding_count: bindings.len(),
        }
    }
}
// impl<'a> From<&ShaderDataLayout<'a>> for celeritas_sys::ShaderDataLayout {
//     fn from(value: &ShaderDataLayout) -> Self {
//         todo!()
//     }
// }

// --- types

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum PrimitiveTopology {
    Point,
    Line,
    Triangle,
}
impl From<celeritas_sys::PrimitiveTopology> for PrimitiveTopology {
    fn from(value: celeritas_sys::PrimitiveTopology) -> Self {
        match value {
            celeritas_sys::PrimitiveTopology_PRIMITIVE_TOPOLOGY_POINT => PrimitiveTopology::Point,
            celeritas_sys::PrimitiveTopology_PRIMITIVE_TOPOLOGY_LINE => PrimitiveTopology::Line,
            celeritas_sys::PrimitiveTopology_PRIMITIVE_TOPOLOGY_TRIANGLE => {
                PrimitiveTopology::Triangle
            }
            _ => unreachable!("enum conversion should be infallible"),
        }
    }
}
