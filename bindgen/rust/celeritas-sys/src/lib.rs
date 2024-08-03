#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use serde::{Deserialize, Serialize};

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));


impl Default for ShaderBinding {
    fn default() -> Self {
        Self { label: todo!(),
          kind: ShaderBindingKind_BINDING_COUNT,
          vis: ShaderVisibility_VISIBILITY_VERTEX,
          data: todo!()
        }
    }
}

impl Default for ShaderDataLayout {
    fn default() -> Self {
        Self { bindings: [ShaderBinding::default(); 8], binding_count: 0 }
    }
}