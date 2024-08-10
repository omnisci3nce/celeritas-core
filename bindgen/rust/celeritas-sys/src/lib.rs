#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::ffi::c_void;

use serde::{Deserialize, Serialize};

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

// --- Conversions
pub mod conversions {
    use crate::{Mat4, Vec3, Vec4};

    impl From<Vec3> for glam::Vec3 {
        fn from(v: Vec3) -> Self {
            Self {
                x: v.x,
                y: v.y,
                z: v.z,
            }
        }
    }
    impl From<glam::Vec3> for Vec3 {
        fn from(v: glam::Vec3) -> Self {
            Self {
                x: v.x,
                y: v.y,
                z: v.z,
            }
        }
    }

    impl From<Vec4> for glam::Vec4 {
        fn from(v: Vec4) -> Self {
            Self::new(v.x, v.y, v.z, v.w)
        }
    }
    impl From<glam::Vec4> for Vec4 {
        fn from(v: glam::Vec4) -> Self {
            Vec4 {
                x: v.x,
                y: v.y,
                z: v.z,
                w: v.w,
            }
        }
    }

    impl From<Mat4> for glam::Mat4 {
        fn from(m: Mat4) -> Self {
            Self {
                x_axis: glam::Vec4::new(m.data[0], m.data[1], m.data[2], m.data[3]),
                y_axis: glam::Vec4::new(m.data[4], m.data[5], m.data[6], m.data[7]),
                z_axis: glam::Vec4::new(m.data[8], m.data[9], m.data[10], m.data[11]),
                w_axis: glam::Vec4::new(m.data[12], m.data[13], m.data[14], m.data[15]),
            }
        }
    }
    impl From<glam::Mat4> for Mat4 {
        fn from(m: glam::Mat4) -> Self {
            let mut slf = Self { data: [0.0; 16] };
            m.write_cols_to_slice(&mut slf.data);
            slf
        }
    }
}

impl Transform {
    #[inline]
    pub fn identity() -> Self {
        Self {
            position: Vec3 {
                x: 0.,
                y: 0.,
                z: 0.,
            },
            rotation: Vec4 {
                x: 0.,
                y: 0.,
                z: 0.,
                w: 1.0,
            },
            scale: 1.,
            is_dirty: true,
        }
    }
}

impl Default for ShaderBinding {
    fn default() -> Self {
        Self {
            label: "static".as_ptr() as *const _,
            kind: ShaderBindingKind_BINDING_COUNT,
            vis: ShaderVisibility_VISIBILITY_VERTEX,
            data: ShaderBinding__bindgen_ty_1 {
                bytes: ShaderBinding__bindgen_ty_1__bindgen_ty_1 {
                    size: 0,
                    data: std::ptr::null_mut(),
                },
            },
        }
    }
}

impl Default for ShaderDataLayout {
    fn default() -> Self {
        Self {
            bindings: [ShaderBinding::default(); 8],
            binding_count: 0,
        }
    }
}

impl Default for Camera {
    fn default() -> Self {
        let camera_pos = Vec3 {
            x: 18.9,
            y: 10.6,
            z: 11.6,
        };
        let camera_front = Vec3 {
            x: -0.6,
            y: -0.2,
            z: -0.7,
        };
        let camera = unsafe {
            Camera_Create(
                camera_pos,
                camera_front,
                Vec3 {
                    x: 0.0,
                    y: 1.0,
                    z: 0.0,
                },
                45.0,
            )
        };
        camera
    }
}
