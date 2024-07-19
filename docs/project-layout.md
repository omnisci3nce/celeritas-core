---
title: Project Structure
---

```
deps/ - third-party dependencies
docs/ - these docs you're reading now that get built with mkdocs
src/
    core/     - core game engine facilities
    logos/    -
    maths/
    platform/
    ral/
    render/
    resources/
    std/
    systems/
    ui/
```


#### Core

Core holds specifically functionality vital to making games or 3D applications. Contrast this with `std` which contains
code that could form the base layer of almost any software out there.

#### Std

Data structures, algorithms, memory management, etc - all code here forms a foundation for everything above it and can conceivably
be reused in non-game applications.