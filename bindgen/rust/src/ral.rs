//! Wrapper around the RAL code in celeritas-core

use celeritas_sys::{
    BufferHandle, GPU_Buffer, GPU_CmdEncoder, GPU_CmdEncoder_BeginRender, GPU_CmdEncoder_EndRender, GPU_GetDefaultEncoder
};

pub struct FrameRenderEncoder(*mut GPU_CmdEncoder);

/// Holds a pointer into the raw `GPU_Renderpass`
pub struct RenderPass(*mut celeritas_sys::GPU_Renderpass);

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
          celeritas_sys::PrimitiveTopology_PRIMITIVE_TOPOLOGY_TRIANGLE => PrimitiveTopology::Triangle,
          _ => unreachable!("enum conversion should be infallible")
        }
    }
}