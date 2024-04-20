set_project("celeritas")
set_version("0.1.0")
set_config("cc", "gcc")

add_rules("mode.debug", "mode.release") -- we have two modes: debug & release

add_syslinks("m") -- these are must have when compiling

-- -Wall     : base set of warnings
-- -Wextra   : additional warnings not covered by -Wall
-- -Wundef   : undefined macros
-- -Wdouble-promotion : catch implicit converion of float to double
add_cflags("-Wall", "-Wextra", "-Wundef", "-Wdouble-promotion")

if is_mode("debug") then
    add_cflags("-g") -- Add debug symbols in debug mode
elseif is_mode("release") then
    add_defines("CRELEASE")
end


-- Platform defines and system packages
if is_plat("linux") then
    add_defines("CEL_PLATFORM_LINUX")
    add_syslinks("dl", "X11", "pthread")
elseif is_plat("windows") then
    add_defines("CEL_PLATFORM_WINDOWS")
elseif is_plat("macosx") then
    add_defines("CEL_PLATFORM_MAC")
    add_frameworks("Cocoa", "IOKit", "CoreVideo", "OpenGL")
    set_runenv("MTL_DEBUG_LAYER", "1")
    -- add_syslinks("GL")
end

-- Compile GLFW from source
package("local_glfw")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "deps/glfw-3.3.8"))
    on_install(function (package)
        local configs = {}
        -- NOTE(omni): Keeping these around as comments in case we need to modify glfw flags
        -- table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        -- table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
    on_test(function (package)
        -- assert(package:has_cfuncs("add", {includes = "foo.h"}))
    end)
package_end()

add_requires("local_glfw")

local core_sources = {
    "deps/glad/src/glad.c",
    "src/*.c",
    "src/logos/*.c",
    "src/platform/*.c",
    "src/renderer/*.c",
    "src/renderer/backends/*.c",
    "src/resources/*.c",
    "src/std/*.c",
    "src/std/containers/*.c",
    "src/systems/*.c",
}

-- common configuration for both static and shared libraries
target("core_config")
    set_kind("static") -- kind is required but you can ignore it since it's just for common settings
    add_syslinks("dl")
    add_packages("local_glfw")
    add_includedirs("deps/glfw-3.3.8/include/GLFW", {public = true})
    add_includedirs("deps/glad/include", {public = true})
    add_includedirs("deps/stb_image", {public = true})
    add_includedirs("deps/stb_image_write", {public = true})
    add_includedirs("deps/stb_truetype", {public = true})
    add_includedirs("src/", {public = true})
    add_includedirs("src/logos/", {public = true})
    add_includedirs("src/maths/", {public = true})
    add_includedirs("src/platform/", {public = true})
    add_includedirs("src/renderer/", {public = true})
    add_includedirs("src/renderer/backends/", {public = true})
    add_includedirs("src/resources/", {public = true})
    add_includedirs("src/std/", {public = true})
    add_includedirs("src/std/containers", {public = true})
    add_includedirs("src/systems/", {public = true})
    add_files("src/empty.c") -- for some reason we need this on Mac so it doesnt call 'ar' with no files and error
    set_default(false) -- prevents standalone building of this target

target("core_static")
    set_kind("static")
    add_deps("core_config") -- inherit common configurations
    set_policy("build.merge_archive", true)
    add_files(core_sources)

target("core_shared")
    set_kind("shared")
    add_deps("core_config") -- inherit common configurations
    add_files(core_sources)

target("main_loop")
    set_kind("binary")
    set_group("examples")
    add_deps("core_shared")
    add_files("examples/main_loop/ex_main_loop.c")
    set_rundir("$(projectdir)")

target("std")
    set_kind("binary")
    set_group("examples")
    add_deps("core_static")
    add_files("examples/standard_lib/ex_std.c")
    set_rundir("$(projectdir)")

target("obj")
    set_kind("binary")
    set_group("examples")
    add_deps("core_static")
    add_files("examples/obj_loading/ex_obj_loading.c")
    set_rundir("$(projectdir)")

target("transforms")
    set_kind("binary")
    set_group("examples")
    add_deps("core_shared")
    add_files("examples/transforms/ex_transforms.c")
    set_rundir("$(projectdir)")

target("demo")
    set_kind("binary")
    set_group("examples")
    add_deps("core_static")
    add_files("examples/demo/demo.c")
    set_rundir("$(projectdir)")
