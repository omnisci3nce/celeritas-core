use std::env;
use std::path::PathBuf;

fn main() {
    // Tell cargo to look for shared libraries in the specified directory
    // TODO: we need to look based on OS
    println!("cargo:rustc-link-search=../../../build/macosx/arm64/debug");
    // println!("cargo:rustc-link-search=../../build/windows/x64/debug");

    // Tell cargo to tell rustc to link the system bzip2
    // shared library.
    println!("cargo:rustc-link-lib=core_static");
    println!("cargo:rustc-link-lib=glfw3");
    // TODO: ^ use our locally compiled glfw

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header("../../../include/amalgamation.h")
        // -- our code
        .clang_arg("-I../../../src")
        .clang_arg("-I../../../src/core")
        .clang_arg("-I../../../src/maths")
        .clang_arg("-I../../../src/new_render")
        .clang_arg("-I../../../src/platform")
        .clang_arg("-I../../../src/ral")
        .clang_arg("-I../../../src/ral/backends/opengl")
        .clang_arg("-I../../../src/resources")
        .clang_arg("-I../../../src/std")
        .clang_arg("-I../../../src/std/containers")
        .clang_arg("-I../../../src/systems")
        // -- dependencies
        .clang_arg("-I../../../deps/cgltf")
        .clang_arg("-I../../../deps/glfw-3.3.8/include/GLFW")
        .clang_arg("-I../../../deps/glad/include")
        .clang_arg("-I../../../deps/stb_image")
        .clang_arg("-I../../../deps/stb_image_write")
        .clang_arg("-I../../../deps/stb_truetype")
        .generate_inline_functions(true)
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
