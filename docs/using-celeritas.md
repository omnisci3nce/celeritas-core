---
title: Using Celeritas
---

**TL;DR**  

- use the amalgamation header `celeritas.h` that exposes all public functions in a single header
    - _you can recreate it with_ `scripts/amalgamation/gen_header.py`
- link the precompiled `core_static` or `core_shared` libraries
- _or_ compile using `xmake build`, the binaries will be in `build/<os>/x64/core_static`, etc