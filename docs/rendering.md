# Rendering

Rendering is split into 3 'registers'.

1. **RAL** (Render Abstraction Layer) - thin abstraction over graphics APIs
2. **render** - implements the default renderer and higher-level functions
3. **immediate** - immediate-mode drawing API for things like debug visualisation and UI