# celeritas-core

Celeritas is an engine written in C that acts as an extendable base layer for creating custom game-specific "engines" (frameworks) that includes helpful wrappers around input handling, cameras, rendering abstractions, and other helpful APIs
for making games and binding to other languages' FFIs.

![Backpack model with lighting](examples/obj_loading/backpack_screenshot.png)

**Work-in-progress**: This code is currently in flux as I experiment with APIs and is ultimately intended to support a 3D game I'm in the process of making.

All third-party dependencies are licensed under their own license.

## Developing

#### Handy commands

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
