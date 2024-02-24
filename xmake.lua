set_project("celeritas")
set_version("0.1.0")
set_config("cc", "gcc")

add_rules("mode.debug", "mode.release") -- we have two modes: debug & release

-- -Wall     : base set of warnings
-- -Wextra   : additional warnings not covered by -Wall
-- -Wundef   : undefined macros
-- -Wdouble-promotion : catch implicit converion of float to double
add_cflags("-Wall", "-Wextra", "-Wundef", "-Wdouble-promotion")

if is_mode("debug") then
    add_cflags("-g") -- Add debug symbols in debug mode
end

-- common configuration for both static and shared libraries
target("core_config")
    set_kind("static") -- kind is required but you can ignore it since it's just for common settings
    add_includedirs("src/", {public = true})
    add_includedirs("src/platform/", {public = true})
    add_files("src/platform/*.c")
    set_default(false) -- prevents standalone building of this target

-- Define a static library
target("core_static")
    set_kind("static")
    add_deps("core_config") -- inherit common configurations

-- Define a shared library
target("core_shared")
    set_kind("shared")
    add_deps("core_config") -- inherit common configurations


target("first")
    set_kind("binary")
    add_deps("core_shared")
    add_files("examples/first.c")
    set_rundir("$(projectdir)")