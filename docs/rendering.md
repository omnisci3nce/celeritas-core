# Rendering

Rendering is split into 3 'registers'.

1. **RAL** (Render Abstraction Layer) - thin abstraction over graphics APIs
2. **Render** - implements the default renderer and higher-level functions
3. **Immediate** - immediate-mode drawing API for things like debug visualisation and UI


## RAL

- RAL doesn't know what 'meshes' are or 'materials', it purely deals with buffers, textures, pipelines, etc. Those concepts
  are left to the `Render` module.