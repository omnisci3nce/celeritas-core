# celeritas-core

![Backpack model with lighting](examples/obj_loading/backpack_screenshot.png)


**Work-in-progress**: This code is currently in flux as I port from OpenGL to a Vulkan backend with a homemade graphics API abstraction layer I'm developing named **RAL** (render abstraction layer) in the source code.

All third-party dependencies are licensed under their own license.

## Notes

* Check symbols in an 'archive' (static library)
    * `nm -C build/libcore.a`

* Generate compiler_commands.json
    * `xmake project -k compile_commands`

* Formatting
    * `xmake format`
    * Lint (no change) `find src/ -iname *.h -o -iname *.c | xargs clang-format --style=file --dry-run --Werror`
    * Format (edit in place) `find src/ \( -iname "*.h" -o -iname "*.c" \) | xargs clang-format -i --style=file`
        * `clang-format` must be installed!
* Documentation
    * serve mkdocs locally
        * `docker run --rm -it -p 8000:8000 -v ${PWD}:/docs squidfunk/mkdocs-material`

    * Build docs static site
        * `docker run --rm -it -v ${PWD}:/docs squidfunk/mkdocs-material build`
