use celeritas::*;

unsafe fn run_game() {
    // init
    Core_Bringup();

    let core = get_global_core();
    let glfw_window_ptr = Core_GetGlfwWindowPtr(core);

    let camera_pos = Vec3 {
        x: 0.0,
        y: 2.0,
        z: -3.0,
    };
    let camera = Camera_Create(
        camera_pos,
        vec3_normalise(vec3_negate(camera_pos)),
        VEC3_Y,
        45.0,
    );
    SetCamera(camera);

    let whatever = Vec3 {
        x: 1.0,
        y: 1.0,
        z: 1.0,
    };
    let sun = DirectionalLight {
        direction: whatever,
        ambient: whatever,
        diffuse: whatever,
        specular: whatever,
    };
    SetMainLight(sun);

    // let skybox = Skybox_Create(face_paths, 6);

    // main loop
    while !ShouldExit() {
        Frame_Begin();

        // Skybox_Draw(&mut skybox, camera);

        Frame_End();
    }
}

fn main() {
    println!("Running from Rust!");

    unsafe {
        run_game();
    }
}
