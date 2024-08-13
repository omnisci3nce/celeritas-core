//! Wrapper around the RAL code in celeritas-core

use std::ffi::c_void;

use celeritas_sys::{
    BufferHandle, GPU_CmdEncoder, GPU_CmdEncoder_BeginRender, GPU_CmdEncoder_EndRender,
    GPU_EncodeBindShaderData, GPU_GetDefaultEncoder, GPU_GetDefaultRenderpass,
    GPU_GraphicsPipeline_Create, GPU_Pipeline, GraphicsPipelineDesc,
    ShaderBindingKind_BINDING_BYTES, ShaderBinding__bindgen_ty_1,
    ShaderBinding__bindgen_ty_1__bindgen_ty_1, ShaderDesc, ShaderVisibility_VISIBILITY_COMPUTE,
    ShaderVisibility_VISIBILITY_FRAGMENT, ShaderVisibility_VISIBILITY_VERTEX, Str8, TextureHandle,
    VertexDescription, MAX_SHADER_DATA_LAYOUTS,
};
use thiserror::Error;

use crate::IntoFFI;

/// Holds a pointer to the raw `GPU_CmdEncoder`
pub struct FrameRenderEncoder(*mut GPU_CmdEncoder);

/// Holds a pointer to the raw `GPU_Renderpass`
pub struct RenderPass(pub *mut celeritas_sys::GPU_Renderpass);

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

// Vertex Descriptions
pub enum VertexAttrKind {
    Floatx1,
    Floatx2,
    Floatx3,
    Floatx4,
    U32x1,
    U32x2,
    U32x3,
    U32x4,
    I32x1,
    I32x2,
    I32x3,
    I32x4,
}

pub struct VertexAttribute {
    name: String,
    kind: VertexAttrKind,
}

#[derive(Default)]
pub struct VertexDesc {
    debug_label: String,
    attributes: Vec<VertexAttribute>,
}
impl VertexDesc {
    pub fn new(name: String) -> Self {
        Self {
            debug_label: name,
            attributes: vec![],
        }
    }
    pub fn add_attr(mut self, attr_name: &str, kind: VertexAttrKind) -> Self {
        self.attributes.push(VertexAttribute {
            name: attr_name.to_owned(),
            kind,
        });
        self
    }
}
impl IntoFFI for VertexDesc {
    type FFIType = VertexDescription;

    unsafe fn into_ffi(self) -> Self::FFIType {
        VertexDescription {
            debug_label: todo!(),
            attr_names: todo!(),
            attributes: todo!(),
            attributes_count: todo!(),
            use_full_vertex_size: todo!(),
        }
    }
}

pub struct PipelineBuilder {
    name: String,
    renderpass: Option<RenderPass>,
    vertex_description: VertexDesc,
    data_layouts: Vec<ShaderDataLayout>,
    shader_paths: Option<(String, String)>,
}

#[derive(Debug, Error)]
pub enum RALError {
    #[error("exceeded maximum of 8 layouts for a pipeline")]
    TooManyShaderDataLayouts,
}

impl PipelineBuilder {
    /// Create a new [PipelineBuilder]
    pub fn new(name: String) -> Self {
        let vertex_description = VertexDesc::new(format!("{} Vertex Description", name.clone()));
        Self {
            name,
            renderpass: None,
            vertex_description,
            data_layouts: Vec::new(),
            shader_paths: None,
        }
    }
    pub fn build(self) -> Result<Pipeline, RALError> {
        let layouts = [celeritas_sys::ShaderDataLayout::default(); 8];
        if self.data_layouts.len() > MAX_SHADER_DATA_LAYOUTS as usize {
            return Err(RALError::TooManyShaderDataLayouts);
        }
        for (i, layout) in self.data_layouts.iter().enumerate().take(8) {
            // layouts[i] = celeritas_sys::ShaderDataLayout::from(layout);
        }
        let (vert_path, frag_path) = self.shader_paths.expect("Shader paths must be provided");
        let vert_code = std::fs::read_to_string(vert_path.clone()).expect("msg");
        let frag_code = std::fs::read_to_string(frag_path.clone()).expect("msg");

        // TODO: convert VertexDesc -> ffi::VertexDescription
        // load shader
        let vs = ShaderDesc {
            debug_name: "".as_ptr() as *const i8,
            filepath: Str8::from_str(&vert_path),
            code: Str8::from_str(&vert_code),
            is_spirv: false,
            is_combined_vert_frag: false,
        };
        let fs = ShaderDesc {
            debug_name: "".as_ptr() as *const i8,
            filepath: Str8::from_str(&frag_path),
            code: Str8::from_str(&frag_code),
            is_spirv: false,
            is_combined_vert_frag: false,
        };

        let desc = GraphicsPipelineDesc {
            debug_name: "".as_ptr() as *const _,
            vertex_desc: unsafe { self.vertex_description.into_ffi() },
            vs,
            fs,
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

    pub fn add_vertex_desc(mut self, vertex_desc: VertexDesc) -> Self {
        self.vertex_description = vertex_desc;
        self
    }
    pub fn add_shader_layout<S: ShaderData>(mut self) -> Self {
        let layout = S::layout();
        self.data_layouts.push(layout);
        self
    }
    pub fn add_shader_src(mut self, vertex_path: &str, fragment_path: &str) -> Self {
        self.shader_paths = Some((vertex_path.to_owned(), fragment_path.to_owned()));
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
    pub fn from_slice(bindings: &[ShaderBinding]) -> Self {
        todo!()
    }
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
            celeritas_sys::PrimitiveTopology_CEL_POINT => PrimitiveTopology::Point,
            celeritas_sys::PrimitiveTopology_CEL_LINE => PrimitiveTopology::Line,
            celeritas_sys::PrimitiveTopology_CEL_TRI => {
                PrimitiveTopology::Triangle
            }
            _ => unreachable!("enum conversion should be infallible"),
        }
    }
}
#[cfg(test)]
mod test {
    use super::*;

    struct TestData {
        _a: [f32; 2],
        _b: [f32; 4],
    }
    impl ShaderData for TestData {
        fn layout() -> ShaderDataLayout {
            todo!()
        }

        fn bind(&self) {
            todo!()
        }
    }

    #[test]
    fn typecheck_pipeline_create() {
        let vertex_desc = VertexDesc::new("Empty".into())
            .add_attr("position", VertexAttrKind::Floatx2)
            .add_attr("color", VertexAttrKind::Floatx4);

        let builder = PipelineBuilder::new("Test Pipeline".into())
            .add_shader_layout::<TestData>()
            .add_vertex_desc(vertex_desc);

        let pipeline = builder.build().expect("Should be valid");
    }
}
