# celeritas-core

Celeritas is an engine 'core' written in C that acts as an extendable base layer for creating custom game-specific "engines" (frameworks) or other
multimedia libraries.

Celeritas Core focuses on rendering and platform abstractions, helpful wrappers around input handling, and common components needed by 3D game or interactive applications such as cameras, collision detection, etc.

Bindings to Rust are WIP and other languages are planned for the future.

Higher-level concepts can then be overlaid on top of this core!

All third-party dependencies are licensed under their own license.

## Goals

- Easy to compile
- Headless build + tests for reliability
- Bindgen
- Targets Vulkan 1.3 for simpler code
- Plugin system (?)
- We can build tools on top of this layer while maintaining high performance. The C side retains
  all rendering data.

Renderer Goals:

- Cascading Shadow Maps
- PBR (Filament)
- GI
- Forward+
- Water
- Terrain

## Developing

#### Handy commands

- Check symbols in an 'archive' (static library)
  - `nm -C build/libcore.a`
- Generate compiler_commands.json
  - `bear -- make`
