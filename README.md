# celeritas-core

Celeritas is an engine 'core' written in C that acts as an extendable base layer for creating custom game-specific "engines" (frameworks). It includes helpful wrappers around input handling, cameras, rendering abstractions, and other helpful APIs for making games and binding to other languages' FFIs.

Higher-level concepts can then be overlaid on top of this core!

![Backpack model with lighting](examples/obj_loading/backpack_screenshot.png)

**Work-in-progress**: This code is currently in flux as I experiment with APIs and is ultimately intended to support a 3D game I'm in the process of making.

All third-party dependencies are licensed under their own license.

## Project Structure

```
/bindgen - bindings code generation for other languages
/deps    - third-party dependencies
/docs    - high-level documentation
/scripts
```

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

## TODO

#### Engine
- [ ] Embedded assets - shaders, textures, et al (probably will use C23's `#embed`?)
- [ ] Shader hot-reloading
- [ ] Cross-platform threadpool
- Strings
  - [x] custom fat pointer string type
- [ ] Transform gizmo

#### Memory
- [x] Arena allocator
  - [x] malloc backed
  - [ ] overcommit address-space backed (`VirtualAlloc` & `mmap`)
- [x] Pool allocator (typed)
  - [ ] Generational handles
- [ ] SoA hot/cold pool allocator (pool for all entities of same type, split into two structs in SoA so we can have hot ,(`VkHandle`and cold `size`, `format` data separated))

#### Scene
- [ ] Transform hierarchy / Scene tree
  - [ ] Transform propagation
- [ ] Asset streaming

### Renderer
- [ ] PBR
  - [x] Basic implementation using learnopengl
  - [ ] Implementation using filament as a reference for first in class PBR
  - [ ] Handle metallic / roughness being in different channels, combined, or absent
- [ ] Shadows
  - [x] Shadowmaps
  - [ ] PCF
  - [ ] Cascading shadowmaps (CSM)
  - [ ] Point light shadows
- [ ] Resizing viewport
- [ ] Cel shading
- [ ] Terrain
  - [ ] Heightmaps
- [ ] Water
  - [ ] water plane
- [ ] Animation
  - [x] Joint and keyframe loading
  - [ ] Handle multiple animations in one GLTF
  - [ ] Animation Blending
- [ ] Global illumination (future)

### RAL
- [x] Buffer/texture creation

### Logistics

- [ ] Replace screenshot with one using PBR + skybox + shadows
- [ ] Update website
- [ ] Check licenses of assets currently in `assets` folder