---
title: Naming Conventions
---

#### 1. Prefer SOV function names

Prefer SOV Subject Object Verb naming for functions.

This makes it very easy to find the functions you want with autocomplete and maintain a consistent naming convention
throughout the codebase.

e.g.

* `renderer_frame_begin`
* `engine_tick_begin`
* `texture_data_load`

---

#### 2. Long-running systems

in celeritas a "system" is roughly something that runs every frame

systems that run for the lifetime of the application or for a very long time should have:

* `bool system_init(system_state* state)` a `init` function
* `void system_shutdown(system_state* state)` and a `shutdown` function

---

#### 3. TODO
