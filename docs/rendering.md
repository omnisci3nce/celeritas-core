# Rendering

Rendering is split into 3 'registers'.

1. **RAL** (Render Abstraction Layer) - thin abstraction over graphics APIs
2. **Render** - implements the default renderer and higher-level functions
3. **Immediate** - immediate-mode drawing API for things like debug visualisation and UI


## RAL

- RAL doesn't know what 'meshes' are or 'materials', it purely deals with buffers, textures, pipelines, etc. Those concepts
  are left to the `Render` module.

## Render

## Adding shader effects

A few decisions have affected how shader data binding works in the RAL and if you're coming from purely OpenGL with floating uniforms like

```glsl
uniform mat4 viewProjection;
```
it might feel a little complicated. However, this setup helps us abstract over different graphics API backends as well as be more performant in the general case
with more data uploaded in one call.
Because of the lack of generics in C, we adopt a `void*` data with a function pointer returning a "layout" for bindings that the backend can then interpret in order
to know how to bind that specific data. This means you will need to do a little bit of work creating a function for each shader data binding. We'll look at an example:

**TODO: ShaderBinding / Layout example**