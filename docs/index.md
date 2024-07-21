# Welcome to Celeritas

Welcome to the Celeritas Game Engine's documentation!

## What is it?

Celeritas is a small 3D game engine 'framework' written in plain ol' C **and** OCaml.

The idea underlying celeritas is to have a small versatile "core" that can then be added to from other languages.
This means you could add gameplay systems in OCaml but rely on the celeritas core layer to do all the heavy lifting 
when it comes to computation or rendering. I will be providing ocaml bindings to the C API but implementing Lua bindings for example
would be fairly trivial.

[Riot Games refers to this](https://technology.riotgames.com/news/future-leagues-engine) as **engine-heavy** vs. **engine-light**; we are strongly in the engine-light category.

**What does 'celeritas' mean?**

Celerity is an English word meaning "alacrity" "swiftness".
Celeritas is the original Latin word for celerity that the English is derived from.

## Feature Set

[See here (README)](https://github.com/omnisci3nce/celeritas-core/blob/winter-cleaning/README.md#todo)

## Getting started

Check these pages out

* [Getting started](getting-started.md)
* [Project layout](project-layout.md)