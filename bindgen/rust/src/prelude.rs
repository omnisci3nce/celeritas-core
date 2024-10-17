// --- maths types
pub use celeritas_sys::mat4;
pub use celeritas_sys::quat;
pub use celeritas_sys::transform;
pub use celeritas_sys::vec2;
pub use celeritas_sys::vec3;
pub use celeritas_sys::vec4;

// --- handles
pub type BufHandle = celeritas_sys::buf_handle;
pub type TexHandle = celeritas_sys::tex_handle;
// pub type MaterialHandle = celeritas_sys::material_handle;
// pub type MeshHandle = celeritas_sys::mesh_handle;
pub type ModelHandle = celeritas_sys::model_handle;
pub type PipelineHandle = celeritas_sys::pipeline_handle;
// pub type PipelineLayoutHandle = celeritas_sys::pipeline_layout_handle;
// pub type RenderpassHandle = celeritas_sys::renderpass_handle;

// // --- conversions
// pub use celeritas_sys::conversions;
