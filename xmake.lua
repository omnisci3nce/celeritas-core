set_project("celeritas")
set_version("0.1.0")
set_config("cc", "clang")

add_rules("mode.debug", "mode.release") -- we have two modes: debug & release

-- -Wall     : base set of warnings
-- -Wextra   : additional warnings not covered by -Wall
-- -Wundef   : undefined macros
-- -Wdouble-promotion : catch implicit converion of float to double
add_cflags("-Wall", "-Wextra", "-Wundef", "-Wdouble-promotion")

if is_mode("debug") then
    add_cflags("-g") -- Add debug symbols in debug mode
    add_defines("CDEBUG")
elseif is_mode("release") then
    add_defines("CRELEASE")
end

-- Platform defines and system packages
if is_plat("linux") then
    add_defines("CEL_PLATFORM_LINUX")
    add_syslinks("dl", "X11", "pthread", "vulkan")
elseif is_plat("windows") then
    add_defines("CEL_PLATFORM_WINDOWS")
    add_syslinks("user32", "gdi32", "kernel32", "shell32")
    add_requires("vulkansdk", { system = true })

    -- add_links("pthreadVC2-w64")
elseif is_plat("macosx") then
    add_defines("CEL_PLATFORM_MAC")
    add_frameworks("Cocoa", "IOKit", "CoreVideo", "OpenGL")
    add_frameworks("Foundation", "Metal", "QuartzCore")
    set_runenv("MTL_DEBUG_LAYER", "1")
    -- add_syslinks("GL")
end

-- Compile GLFW from source
package("local_glfw")
add_deps("cmake")
set_sourcedir(path.join(os.scriptdir(), "deps/glfw-3.3.8"))
on_install(function(package)
    local configs = {}
    -- NOTE(omni): Keeping these around as comments in case we need to modify glfw flags
    -- table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
    -- table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
    import("package.tools.cmake").install(package, configs)
end)
on_test(function(package)
    -- assert(package:has_cfuncs("add", {includes = "foo.h"}))
end)
package_end()

add_requires("local_glfw")

local core_sources = {
    "deps/glad/src/glad.c",
    "src/*.c",
    "src/core/*.c",
    -- "src/logos/*.c",
    "src/maths/*.c",
    "src/platform/*.c",
    "src/physics/*.c",
    "src/ral/*.c",
    -- "src/ral/backends/opengl/*.c",
    "src/new_render/*.c",
    -- "src/renderer/*.c",
    -- "src/renderer/backends/*.c",
    -- "src/renderer/backends/opengl/*.c",
    "src/resources/*.c",
    "src/std/*.c",
    "src/std/containers/*.c",
    "src/systems/*.c",
}

local unity_sources = {
    "deps/Unity/src/unity.c",
    "deps/Unity/extras/fixture/src/unity_fixture.c",
    "deps/Unity/extras/memory/src/unity_memory.c",
}

rule("compile_glsl_vert_shaders")
set_extensions(".vert")
on_buildcmd_file(function(target, batchcmds, sourcefile, opt)
    local targetfile = path.join(target:targetdir(), path.basename(sourcefile) .. ".vert.spv")

    print("Compiling shader: %s to %s", sourcefile, targetfile)
    batchcmds:vrunv('glslc', { sourcefile, "-o", targetfile })
    -- batchcmds:add_depfiles(sourcefile)
end)
rule("compile_glsl_frag_shaders")
set_extensions(".frag")
on_buildcmd_file(function(target, batchcmds, sourcefile, opt)
    local targetfile = path.join(target:targetdir(), path.basename(sourcefile) .. ".frag.spv")

    print("Compiling shader: %s to %s", sourcefile, targetfile)
    batchcmds:vrunv('glslc', { sourcefile, "-o", targetfile })
    -- batchcmds:add_depfiles(sourcefile)
end)
-- TODO: Metal shaders compilation

-- common configuration for both static and shared libraries
target("core_config")
set_kind("static") -- kind is required but you can ignore it since it's just for common settings
add_packages("local_glfw")
add_defines("CEL_REND_BACKEND_OPENGL")
add_includedirs("deps/cgltf", { public = true })
add_includedirs("deps/glfw-3.3.8/include/GLFW", { public = true })
add_includedirs("deps/glad/include", { public = true })
add_includedirs("deps/stb_image", { public = true })
add_includedirs("deps/stb_image_write", { public = true })
add_includedirs("deps/stb_truetype", { public = true })
add_includedirs("include/", { public = true })
add_includedirs("src/", { public = true })
add_includedirs("src/core", { public = true })
-- add_includedirs("src/logos/", {public = true})
add_includedirs("src/maths/", { public = true })
add_includedirs("src/platform/", { public = true })
add_includedirs("src/physics/", { public = true })
add_includedirs("src/ral", { public = true })
add_includedirs("src/ral/backends/opengl", { public = true })
add_includedirs("src/new_render", { public = true })
-- add_includedirs("src/renderer/", {public = true})
-- add_includedirs("src/renderer/backends/", {public = true})
-- add_includedirs("src/renderer/backends/opengl", {public = true})
-- add_includedirs("src/renderer/backends/metal", {public = true})
add_includedirs("src/resources/", { public = true })
add_includedirs("src/std/", { public = true })
add_includedirs("src/std/containers", { public = true })
add_includedirs("src/systems/", { public = true })
add_files("src/empty.c") -- for some reason we need this on Mac so it doesnt call 'ar' with no files and error
-- add_rules("compile_glsl_vert_shaders")
-- add_rules("compile_glsl_frag_shaders")
-- add_files("assets/shaders/triangle.vert")
-- add_files("assets/shaders/triangle.frag")
-- add_files("assets/shaders/cube.vert")
-- add_files("assets/shaders/cube.frag")
-- add_files("assets/shaders/*.frag")
if is_plat("windows") then
    add_includedirs("$(env VULKAN_SDK)/Include", { public = true })
    add_linkdirs("$(env VULKAN_SDK)/Lib", { public = true })
    add_links("vulkan-1")
end
if is_plat("macosx") then
    add_files("src/renderer/backends/metal/*.m")
end
set_default(false) -- prevents standalone building of this target

target("core_static")
set_kind("static")
add_deps("core_config") -- inherit common configurations
set_policy("build.merge_archive", true)
add_files(core_sources)
-- Link against static CRT
if is_plat("windows") then
    add_links("libcmt", "legacy_stdio_definitions")  -- for release builds
    add_links("libcmtd", "legacy_stdio_definitions") -- for debug builds
end

target("core_shared")
set_kind("shared")
add_deps("core_config")     -- inherit common configurations
add_files(core_sources)
-- Link against dynamic CRT
if is_plat("windows") then
    add_links("msvcrt", "legacy_stdio_definitions")      -- for release builds
    add_links("msvcrtd", "legacy_stdio_definitions")     -- for debug builds
end

-- target("main_loop")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_static")
--     add_files("examples/main_loop/ex_main_loop.c")
--     set_rundir("$(projectdir)")

-- target("tri")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_static")
--     add_files("examples/triangle/ex_triangle.c")
--     set_rundir("$(projectdir)")
--     if is_plat("macosx") then
--         before_build(function (target)
--             print("build metal shaders lib")
--             os.exec("mkdir -p build/shaders")
--             os.exec("xcrun -sdk macosx metal -c assets/shaders/triangle.metal -o build/shaders/gfx.air")
--             os.exec("xcrun -sdk macosx metallib build/shaders/gfx.air -o build/gfx.metallib")
--         end)
--     end

-- target("cube")
--     set_kind("binary")
--     set_group("examples")
--     -- add_defines("CEL_REND_BACKEND_OPENGL")
--     add_deps("core_static")
--     add_files("examples/cube/ex_cube.c")
--     set_rundir("$(projectdir)")

-- target("primitives")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_static")
--     add_files("examples/primitives/ex_primitives.c")
--     set_rundir("$(projectdir)")

-- -- target("std")
-- --     set_kind("binary")
-- --     set_group("examples")
-- --     add_deps("core_static")
-- --     add_files("examples/standard_lib/ex_std.c")
-- --     set_rundir("$(projectdir)")

-- -- target("obj")
-- --     set_kind("binary")
-- --     set_group("examples")
-- --     add_deps("core_static")
-- --     add_files("examples/obj_loading/ex_obj_loading.c")
-- --     set_rundir("$(projectdir)")

-- -- target("input")
-- --     set_kind("binary")
-- --     set_group("examples")
-- --     add_deps("core_static")
-- --     add_files("examples/input/ex_input.c")
-- --     set_rundir("$(projectdir)")

-- target("gltf")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_static")
--     add_files("examples/gltf_loading/ex_gltf_loading.c")
--     set_rundir("$(projectdir)")

-- target("pbr_params")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_static")
--     add_files("examples/pbr_params/ex_pbr_params.c")
--     set_rundir("$(projectdir)")

-- target("pbr_textured")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_static")
--     add_files("examples/pbr_textured/ex_pbr_textured.c")
--     set_rundir("$(projectdir)")

-- target("shadows")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_static")
--     add_files("examples/shadow_maps/ex_shadow_maps.c")
--     set_rundir("$(projectdir)")

-- target("transforms")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_shared")
--     add_files("examples/transforms/ex_transforms.c")
--     set_rundir("$(projectdir)")

-- target("animation")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_shared")
--     add_files("examples/property_animation/ex_property_animation.c")
--     set_rundir("$(projectdir)")

-- target("skinned")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_shared")
--     add_files("examples/skinned_animation/ex_skinned_animation.c")
--     set_rundir("$(projectdir)")

-- target("input")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_static")
--     add_files("examples/input/ex_input.c")
--     set_rundir("$(projectdir)")

-- target("demo")
--     set_kind("binary")
--     set_group("examples")
--     add_deps("core_static")
--     add_files("examples/demo/demo.c")
--     set_rundir("$(projectdir)")

target("game")
set_kind("binary")
set_group("examples")
add_deps("core_static")
add_files("examples/game_demo/game_demo.c")
set_rundir("$(projectdir)")

-- target("pool_tests")
--     set_kind("binary")
--     set_group("tests")
--     add_deps("core_static")
--     add_files(unity_sources)
--     add_includedirs("deps/Unity/src", {public = true})
--     add_includedirs("deps/Unity/extras/fixture/src", {public = true})
--     add_includedirs("deps/Unity/extras/memory/src", {public = true})
--     add_files("tests/pool_tests.c")
--     add_files("tests/pool_test_runner.c")
