use std::ffi::CString;
use std::ptr::addr_of_mut;

use celeritas::*;
use egui_glfw as egui_backend;
use egui_backend::egui::{vec2, Pos2, Rect};
use egui_glfw::glfw::{Context, fail_on_errors};

use egui_glfw::glfw;

const SCREEN_WIDTH: u32 = 2000;
const SCREEN_HEIGHT: u32 = 1800;

fn main() {
    unsafe {
      let mut glfw = glfw::init(glfw::fail_on_errors!()).unwrap();
      glfw.window_hint(glfw::WindowHint::ContextVersion(4, 1));
      glfw.window_hint(glfw::WindowHint::OpenGlProfile(
          glfw::OpenGlProfileHint::Core,
      ));
      glfw.window_hint(glfw::WindowHint::DoubleBuffer(true));
      glfw.window_hint(glfw::WindowHint::Resizable(false));
  
      let (mut window, events) = glfw
          .create_window(
              SCREEN_WIDTH,
              SCREEN_HEIGHT,
              "Egui in GLFW!",
              glfw::WindowMode::Windowed,
          )
          .expect("Failed to create GLFW window.");
  
      window.set_all_polling(true);
      window.make_current();
      // glfw.set_swap_interval(glfw::SwapInterval::None);
      glfw.set_swap_interval(glfw::SwapInterval::Adaptive);

      
  
      gl::load_with(|symbol| window.get_proc_address(symbol) as *const _);

      let window_ptr = window.window_ptr();
      unsafe { 
        // Cast the window pointer to the expected type
        let window_ptr = window_ptr as *mut celeritas::GLFWwindow;
        Core_Bringup(window_ptr);
    };
  
      let mut painter = egui_backend::Painter::new(&mut window);
      let egui_ctx = egui::Context::default();
      
  
      let (width, height) = window.get_framebuffer_size();
      let native_pixels_per_point = window.get_content_scale().0;
      let native_pixels_per_point = 1.0;
      // egui_ctx.set_pixels_per_point(2.0);
  
      let mut egui_input_state = egui_backend::EguiInputState::new(egui::RawInput {
          screen_rect: Some(Rect::from_min_size(
              Pos2::new(0f32, 0f32),
              vec2(width as f32, height as f32) / native_pixels_per_point,
          )),
          ..Default::default()
      });
  
      egui_input_state.input.time = Some(0.01);
      
      // let triangle = triangle::Triangle::new();
      let slider = &mut 0.0;

      // C data
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
                let camera = Camera_Create(
                    camera_pos,
                    camera_front,
                    Vec3 {
                        x: 0.0,
                        y: 1.0,
                        z: 0.0,
                    },
                    45.0,
                );
                SetCamera(camera);
        
                let mut cube_geo = Geo_CreateCuboid(f32x3 {
                    x: 2.0,
                    y: 2.0,
                    z: 2.0,
                });
                let mut crate_mesh = Mesh_Create(addr_of_mut!(cube_geo), false);
                let albedo_map = TextureLoadFromFile(
                    CString::new("assets/demo/crate/Wood_Crate_001_basecolor.jpg")
                        .unwrap()
                        .as_ptr() as *const i8,
                );
                let roughness_map = TextureLoadFromFile(
                    CString::new("assets/demo/crate/Wood_Crate_001_roughness.jpg")
                        .unwrap()
                        .as_ptr() as *const i8,
                );
                let normal_map = TextureLoadFromFile(
                    CString::new("assets/demo/crate/Wood_Crate_001_normal.jpg")
                        .unwrap()
                        .as_ptr() as *const i8,
                );
                let ao_map = TextureLoadFromFile(
                    CString::new("assets/demo/crate/Wood_Crate_001_ambientOcclusion.jpg")
                        .unwrap()
                        .as_ptr() as *const i8,
                );
                let name: [i8; 64] = [0; 64];
                let mut crate_mat = Material {
                    name: name,
                    kind: 0,
                    param_albedo: Vec3 {
                        x: 0.0,
                        y: 0.0,
                        z: 0.0,
                    },
                    param_metallic: 0.0,
                    param_roughness: 0.0,
                    param_ao: 0.0,
                    pbr_albedo_map: albedo_map,
                    pbr_normal_map: normal_map,
                    metal_roughness_combined: true,
                    pbr_metallic_map: TextureHandle { raw: 99999 },
                    pbr_roughness_map: roughness_map,
                    pbr_ao_map: ao_map,
                };
                let crate_renderent = RenderEnt {
                    mesh: addr_of_mut!(crate_mesh),
                    material: addr_of_mut!(crate_mat),
                    affine: mat4_ident(),
                    casts_shadows: true,
                };
                let mut render_ents: [RenderEnt; 1] = [crate_renderent];
                let mut skybox = Skybox_Default();
        
  
      // Main rendering loop
      while !window.should_close() {
          glfw.poll_events();
  
          egui_ctx.begin_frame(egui_input_state.input.take());
  
          unsafe {
              gl::ClearColor(0.455, 0.302, 0.663, 1.0);
              gl::Clear(gl::COLOR_BUFFER_BIT);
              gl::Clear(gl::DEPTH_TEST);
          }
  
          Frame_Begin();
          gl::Enable(gl::DEPTH_TEST);
          gl::Enable(gl::CULL_FACE);

          Skybox_Draw(addr_of_mut!(skybox), camera);
            Render_RenderEntities(
                render_ents.as_mut_ptr(),
                render_ents.len(),
            );
            
            // Frame_End();

          // triangle.draw();
  
            gl::Disable(gl::DEPTH_TEST);
            gl::Disable(gl::CULL_FACE);

          egui::Window::new("Egui with GLFW").show(&egui_ctx, |ui| {
              ui.label("Celeritas in-game editor");
              let btn_m = &mut ui.button("-");
              let btn_p = &mut ui.button("+");
  
              ui.add(egui::Slider::new(slider, 0.0..=100.0).text("My value"));
  
              if btn_m.clicked() && *slider > 0.0 {
                  *slider -= 1.0;
              }
  
              if btn_p.clicked() && *slider < 100.0 {
                  *slider += 1.0;
              }
          });
  
          let egui::FullOutput {
              platform_output,
              textures_delta,
              shapes, .. } = egui_ctx.end_frame();
  
          //Handle cut, copy text from egui
          if !platform_output.copied_text.is_empty() {
              egui_backend::copy_to_clipboard(&mut egui_input_state, platform_output.copied_text);
          }
  
          //Note: passing a bg_color to paint_jobs will clear any previously drawn stuff.
          //Use this only if egui is being used for all drawing and you aren't mixing your own Open GL
          //drawing calls with it.
          //Since we are custom drawing an OpenGL Triangle we don't need egui to clear the background.
  
          let clipped_shapes = egui_ctx.tessellate(shapes, native_pixels_per_point);
          painter.paint_and_update_textures(native_pixels_per_point, &clipped_shapes, &textures_delta);
  
          for (_, event) in glfw::flush_messages(&events) {
              match event {
                  glfw::WindowEvent::Close => window.set_should_close(true),
                  _ => {
                      egui_backend::handle_event(event, &mut egui_input_state);
                  }
              }
          }
          
          window.swap_buffers();
      }

    }
}
