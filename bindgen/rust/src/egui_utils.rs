use egui_overlay::{EguiOverlay, OverlayApp};
use egui_render_three_d::ThreeDBackend;
use egui_window_glfw_passthrough::glfw::Context;
use egui_window_glfw_passthrough::{GlfwBackend, GlfwConfig};

use crate::Core_Bringup;

// struct CustomEguiOverlay {
//   backend:
// }

fn init() {
    let mut glfw_backend = GlfwBackend::new(GlfwConfig::default());
    let mut glfw_window_ptr = glfw_backend.window.window_ptr();

    unsafe {
        // Cast the window pointer to the expected type
        let window_ptr = glfw_window_ptr as *mut crate::GLFWwindow;
        Core_Bringup(window_ptr);
    };
}

/// After implementing [`EguiOverlay`], just call this function with your app data
pub fn start<T: EguiOverlay + 'static>(user_data: T) {
    let mut glfw_backend = GlfwBackend::new(GlfwConfig {
        // this closure will be called before creating a window
        glfw_callback: Box::new(|gtx| {
            // some defualt hints. it is empty atm, but in future we might add some convenience hints to it.
            (egui_window_glfw_passthrough::GlfwConfig::default().glfw_callback)(gtx);
            // scale the window size based on monitor scale. as 800x600 looks too small on a 4k screen, compared to a hd screen in absolute pixel sizes.
            gtx.window_hint(egui_window_glfw_passthrough::glfw::WindowHint::ScaleToMonitor(true));
        }),
        opengl_window: Some(true), // macos doesn't support opengl.
        transparent_window: Some(false),
        window_title: "Celeritas egui".into(),
        ..Default::default()
    });

    // always on top
    // glfw_backend.window.set_floating(true);
    // disable borders/titlebar
    // glfw_backend.window.set_decorated(false);

    let latest_size = glfw_backend.window.get_framebuffer_size();
    let latest_size = [latest_size.0 as _, latest_size.1 as _];

    let default_gfx_backend = {
      ThreeDBackend::new(
          egui_render_three_d::ThreeDConfig {
              ..Default::default()
          },
          |s| glfw_backend.get_proc_address(s),
          latest_size,
      )
  };

    let glfw_window_ptr = glfw_backend.window.window_ptr();
    unsafe {
      // Cast the window pointer to the expected type
      let window_ptr = glfw_window_ptr as *mut crate::GLFWwindow;
      Core_Bringup(window_ptr);
    };

    let overlap_app = OverlayApp {
        user_data,
        egui_context: Default::default(),
        default_gfx_backend,
        glfw_backend,
    };

    overlap_app.enter_event_loop();
}
