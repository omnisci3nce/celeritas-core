set_project("celeritas")
set_version("0.1.0")
set_config("cc", "gcc")

add_rules("mode.debug", "mode.release") -- we have two modes: debug & release

-- add_syslinks("m", "dl") -- these are must have when compiling

-- -Wall     : base set of warnings
-- -Wextra   : additional warnings not covered by -Wall
-- -Wundef   : undefined macros
-- -Wdouble-promotion : catch implicit converion of float to double
add_cflags("-Wall", "-Wextra", "-Wundef", "-Wdouble-promotion")

if is_mode("debug") then
    add_cflags("-g") -- Add debug symbols in debug mode
end

-- Platform defines
if is_plat("linux") then
    add_defines("CEL_PLATFORM_LINUX")
elseif is_plat("windows") then
    add_defines("CEL_PLATFORM_WINDOWS")
elseif is_plat("macosx") then
    add_defines("CEL_PLATFORM_MAC")
end

package("glfw")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "glfw"))
    on_install(function (package)
        local configs = {}
        -- table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        -- table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
    on_test(function (package)
        -- assert(package:has_cfuncs("add", {includes = "foo.h"}))
    end)
package_end()

add_requires("glfw")

-- common configuration for both static and shared libraries
target("core_config")
    set_kind("static") -- kind is required but you can ignore it since it's just for common settings
    add_packages("glfw")
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
    add_includedirs("src/std/", {public = true})
    add_includedirs("src/std/containers", {public = true})
    add_includedirs("src/systems/", {public = true})
    add_files("deps/glad/src/glad.c")
    add_files("src/*.c")
    add_files("src/logos/*.c")
    add_files("src/platform/*.c")
    add_files("src/renderer/*.c")
    add_files("src/renderer/backends/*.c")
    add_files("src/std/*.c")
    add_files("src/std/containers/*.c")
    add_files("src/systems/*.c")
    set_default(false) -- prevents standalone building of this target

target("core_static")
    set_kind("static")
    add_deps("core_config") -- inherit common configurations

target("core_shared")
    set_kind("shared")
    add_deps("core_config") -- inherit common configurations

target("main_loop")
    set_kind("binary")
    set_group("examples")
    add_deps("core_shared")
    add_files("examples/main_loop/ex_main_loop.c")
    set_rundir("$(projectdir)")