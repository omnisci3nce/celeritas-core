//! Wrapper around the RAL code in celeritas-core

use std::{os::raw::c_void, ptr};

use celeritas_sys::{
    BufferHandle, GPU_CmdEncoder, GPU_CmdEncoder_BeginRender, GPU_CmdEncoder_EndRender,
    GPU_GetDefaultEncoder, GPU_GetDefaultRenderpass, GPU_GraphicsPipeline_Create,
    GraphicsPipelineDesc, ShaderData,
};

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
}

pub struct PipelineBuilder {
    renderpass: Option<RenderPass>,
    data_layouts: Vec<()>,
}
impl PipelineBuilder {
    // pub fn add_

    pub fn build(self) -> Pipeline {
        let shad = ShaderData {
            get_layout: todo!(),
            data: ptr::null_mut(),
        };
        let desc = GraphicsPipelineDesc {
            debug_name: todo!(),
            vertex_desc: todo!(),
            vs: todo!(),
            fs: todo!(),
            data_layouts: todo!(),
            data_layouts_count: todo!(),
            wireframe: todo!(),
            depth_test: todo!(),
        };
        let p = unsafe {
            GPU_GraphicsPipeline_Create(
                todo!(),
                self.renderpass
                    .map(|r| r.0)
                    .unwrap_or(GPU_GetDefaultRenderpass()),
            )
        };
        Pipeline(p)
    }
}
// impl Default for PipelineBuilder {
//     fn default() -> Self {
//         Self {
//             renderpass: Default::default(),

//         }
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
