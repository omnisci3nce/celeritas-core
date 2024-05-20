---
title: C Coding Style
---

A lot of these should be forced onto you by `clang-format` and `clang-tidy` but for posterity here are
a few of the styles that we stick to.

* **Pointer next to type** - we use `type* variable` in function signatures and variable declarations e.g. `mat4* model`


## Memory

### Arena allocation
