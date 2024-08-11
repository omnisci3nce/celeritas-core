# celeritas-core

Celeritas is an engine 'core' written in C that acts as an extendable base layer for creating custom game-specific "engines" (frameworks).
Celeritas Core focuses on rendering and platform abstractions, helpful wrappers around input handling, and common components needed by 3D game or interactive applications
such as cameras, collision detection, etc.
Bindings to Rust are WIP and other languages are planned for the future.

Higher-level concepts can then be overlaid on top of this core!

![Backpack model with lighting](examples/obj_loading/backpack_screenshot.png)

**Work-in-progress**: This code is currently in flux as I experiment with APIs and is ultimately intended to support a 3D game I'm in the process of making.

All third-party dependencies are licensed under their own license.

## Features

- Log level based logging
- Arena allocator
- Pool allocator
- Physically based rendering (PBR)
- GLTF loading

#### RAL (Render Abstraction Layer)
- [x] Buffer/texture creation
- [x] Graphics pipeline creation/deletion
- [ ] Compute shader pipeline creation/deletion/run

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


#### Logistics

- [ ] Replace screenshot with one using PBR + skybox + shadows
- [ ] Update website
- [ ] Check licenses of assets currently in `assets` folder

## Questions

- How to expose material properties in the immediate mode UI?
