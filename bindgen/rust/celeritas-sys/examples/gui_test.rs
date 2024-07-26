// // #![windows_subsystem = "windows"] // to turn off console.

// use std::ptr;
// use std::{ffi::CString, ptr::addr_of_mut};

// use celeritas::*;
// use egui::DragValue;
// use egui_overlay::{EguiOverlay, OverlayApp};
// use egui_render_three_d::ThreeDBackend;
// use egui_window_glfw_passthrough::glfw::Context;
// use egui_window_glfw_passthrough::{GlfwBackend, GlfwConfig};

// fn main() {
//     // use tracing_subscriber::{fmt, prelude::*, EnvFilter};
//     // if RUST_LOG is not set, we will use the following filters
//     // tracing_subscriber::registry()
//     //     .with(fmt::layer())
//     //     .with(
//     //         EnvFilter::try_from_default_env()
//     //             .unwrap_or(EnvFilter::new("debug,wgpu=warn,naga=warn")),
//     //     )
//     //     .init();

//     unsafe {
//         // let p: *mut GLFWwindow = ptr::null_mut();
//         // Core_Bringup(p);

//         // let core = get_global_core();
//         // let glfw_window_ptr = Core_GetGlfwWindowPtr(core);

//         let mut glfw_backend = GlfwBackend::new(GlfwConfig {
//             // this closure will be called before creating a window
//             glfw_callback: Box::new(|gtx| {
//                 // some defualt hints. it is empty atm, but in future we might add some convenience hints to it.
//                 (egui_window_glfw_passthrough::GlfwConfig::default().glfw_callback)(gtx);
//                 // scale the window size based on monitor scale. as 800x600 looks too small on a 4k screen, compared to a hd screen in absolute pixel sizes.
//                 gtx.window_hint(
//                     egui_window_glfw_passthrough::glfw::WindowHint::ScaleToMonitor(true),
//                 );
//             }),
//             opengl_window: Some(true), // macos doesn't support opengl.
//             transparent_window: Some(false),
//             window_title: "Celeritas egui".into(),
//             ..Default::default()
//         });

//         // always on top
//         // glfw_backend.window.set_floating(true);
//         // disable borders/titlebar
//         // glfw_backend.window.set_decorated(false);

//         let latest_size = glfw_backend.window.get_framebuffer_size();
//         let latest_size = [latest_size.0 as _, latest_size.1 as _];

//         let default_gfx_backend = {
//             ThreeDBackend::new(
//                 egui_render_three_d::ThreeDConfig {
//                     ..Default::default()
//                 },
//                 |s| glfw_backend.get_proc_address(s),
//                 latest_size,
//             )
//         };

//         let glfw_window_ptr = glfw_backend.window.window_ptr();
//         unsafe {
//             // Cast the window pointer to the expected type
//             let window_ptr = glfw_window_ptr as *mut crate::GLFWwindow;
//             Core_Bringup(window_ptr);
//         };

//         // cam pos: 18.871811 10.658584 11.643305 cam frontL -0.644326 -0.209243 -0.735569
//         let camera_pos = Vec3 {
//             x: 18.9,
//             y: 10.6,
//             z: 11.6,
//         };
//         let camera_front = Vec3 {
//             x: -0.6,
//             y: -0.2,
//             z: -0.7,
//         };
//         let camera = Camera_Create(
//             camera_pos,
//             camera_front,
//             Vec3 {
//                 x: 0.0,
//                 y: 1.0,
//                 z: 0.0,
//             },
//             45.0,
//         );
//         SetCamera(camera);

//         let mut cube_geo = Geo_CreateCuboid(f32x3 {
//             x: 2.0,
//             y: 2.0,
//             z: 2.0,
//         });
//         let mut crate_mesh = Mesh_Create(addr_of_mut!(cube_geo), false);
//         let albedo_map = TextureLoadFromFile(
//             CString::new("assets/demo/crate/Wood_Crate_001_basecolor.jpg")
//                 .unwrap()
//                 .as_ptr() as *const i8,
//         );
//         let roughness_map = TextureLoadFromFile(
//             CString::new("assets/demo/crate/Wood_Crate_001_roughness.jpg")
//                 .unwrap()
//                 .as_ptr() as *const i8,
//         );
//         let normal_map = TextureLoadFromFile(
//             CString::new("assets/demo/crate/Wood_Crate_001_normal.jpg")
//                 .unwrap()
//                 .as_ptr() as *const i8,
//         );
//         let ao_map = TextureLoadFromFile(
//             CString::new("assets/demo/crate/Wood_Crate_001_ambientOcclusion.jpg")
//                 .unwrap()
//                 .as_ptr() as *const i8,
//         );
//         let name: [i8; 64] = [0; 64];
//         let mut crate_mat = Material {
//             name: name,
//             kind: 0,
//             param_albedo: Vec3 {
//                 x: 0.0,
//                 y: 0.0,
//                 z: 0.0,
//             },
//             param_metallic: 0.0,
//             param_roughness: 0.0,
//             param_ao: 0.0,
//             pbr_albedo_map: albedo_map,
//             pbr_normal_map: normal_map,
//             metal_roughness_combined: true,
//             pbr_metallic_map: TextureHandle { raw: 99999 },
//             pbr_roughness_map: roughness_map,
//             pbr_ao_map: ao_map,
//         };
//         let crate_renderent = RenderEnt {
//             mesh: addr_of_mut!(crate_mesh),
//             material: addr_of_mut!(crate_mat),
//             affine: mat4_ident(),
//             casts_shadows: true,
//         };
//         let mut render_ents: [RenderEnt; 1] = [crate_renderent];

//         let world_state = HelloWorld {
//             frame: 0,
//             render_entities: render_ents,
//         };

//         let overlap_app = OverlayApp {
//             user_data: world_state,
//             egui_context: Default::default(),
//             default_gfx_backend,
//             glfw_backend,
//         };

//         overlap_app.enter_event_loop();

//         // celeritas::egui_utils::start(HelloWorld { frame: 0});
//     }
// }

// pub struct HelloWorld {
//     pub frame: u64,
//     pub render_entities: [RenderEnt; 1],
// }
// impl EguiOverlay for HelloWorld {
//     fn gui_run(
//         &mut self,
//         egui_context: &egui::Context,
//         _default_gfx_backend: &mut ThreeDBackend,
//         glfw_backend: &mut egui_window_glfw_passthrough::GlfwBackend,
//     ) {
//         unsafe {
//             // Frame_Begin();
//             // Render_RenderEntities(
//             //     self.render_entities.as_mut_ptr(),
//             //     self.render_entities.len(),
//             // );
//             // Frame_End();
//         }

//         // just some controls to show how you can use glfw_backend
//         egui::Window::new("controls").show(egui_context, |ui| {
//             ui.set_width(300.0);
//             self.frame += 1;
//             ui.label(format!("current frame number: {}", self.frame));
//             // sometimes, you want to see the borders to understand where the overlay is.
//             let mut borders = glfw_backend.window.is_decorated();
//             if ui.checkbox(&mut borders, "window borders").changed() {
//                 glfw_backend.window.set_decorated(borders);
//             }

//             ui.label(format!(
//                 "pixels_per_virtual_unit: {}",
//                 glfw_backend.physical_pixels_per_virtual_unit
//             ));
//             ui.label(format!("window scale: {}", glfw_backend.scale));
//             ui.label(format!("cursor pos x: {}", glfw_backend.cursor_pos[0]));
//             ui.label(format!("cursor pos y: {}", glfw_backend.cursor_pos[1]));

//             ui.label(format!(
//                 "passthrough: {}",
//                 glfw_backend.window.is_mouse_passthrough()
//             ));
//             // how to change size.
//             // WARNING: don't use drag value, because window size changing while dragging ui messes things up.
//             let mut size = glfw_backend.window_size_logical;
//             let mut changed = false;
//             ui.horizontal(|ui| {
//                 ui.label("width: ");
//                 ui.add_enabled(false, DragValue::new(&mut size[0]));
//                 if ui.button("inc").clicked() {
//                     size[0] += 10.0;
//                     changed = true;
//                 }
//                 if ui.button("dec").clicked() {
//                     size[0] -= 10.0;
//                     changed = true;
//                 }
//             });
//             ui.horizontal(|ui| {
//                 ui.label("height: ");
//                 ui.add_enabled(false, DragValue::new(&mut size[1]));
//                 if ui.button("inc").clicked() {
//                     size[1] += 10.0;
//                     changed = true;
//                 }
//                 if ui.button("dec").clicked() {
//                     size[1] -= 10.0;
//                     changed = true;
//                 }
//             });
//             if changed {
//                 glfw_backend.set_window_size(size);
//             }
//             // how to change size.
//             // WARNING: don't use drag value, because window size changing while dragging ui messes things up.
//             let mut pos = glfw_backend.window_position;
//             let mut changed = false;
//             ui.horizontal(|ui| {
//                 ui.label("x: ");
//                 ui.add_enabled(false, DragValue::new(&mut pos[0]));
//                 if ui.button("inc").clicked() {
//                     pos[0] += 10;
//                     changed = true;
//                 }
//                 if ui.button("dec").clicked() {
//                     pos[0] -= 10;
//                     changed = true;
//                 }
//             });
//             ui.horizontal(|ui| {
//                 ui.label("y: ");
//                 ui.add_enabled(false, DragValue::new(&mut pos[1]));
//                 if ui.button("inc").clicked() {
//                     pos[1] += 10;
//                     changed = true;
//                 }
//                 if ui.button("dec").clicked() {
//                     pos[1] -= 10;
//                     changed = true;
//                 }
//             });
//             if changed {
//                 glfw_backend.window.set_pos(pos[0], pos[1]);
//             }
//         });

//         // here you decide if you want to be passthrough or not.
//         if egui_context.wants_pointer_input() || egui_context.wants_keyboard_input() {
//             // we need input, so we need the window to be NOT passthrough
//             glfw_backend.set_passthrough(false);
//         } else {
//             // we don't care about input, so the window can be passthrough now
//             glfw_backend.set_passthrough(true)
//         }
//         egui_context.request_repaint();
//     }
// }
