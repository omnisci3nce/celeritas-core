# Welcome to Celeritas

Welcome to the Celeritas Game Engine's documentation!

## What is it?

Celeritas is a small 3D game engine written in OCaml **and** plain ol' C.

The idea underlying celeritas is to have a small versatile "core" that can then be added to from other languages.
This means you could add gameplay systems in OCaml but rely on the celeritas core layer to do all the heavy lifting 
when it comes to computation or rendering.

**What does 'celeritas' mean?**

Celerity is an English word meaning "alacrity" "swiftness".
Celeritas is the original Latin word for celerity that the English is derived from.

## Feature Set

_(as of Jan 2024)_

**Implemented**

* OpenGL renderer backend! ((*need to port Metal renderer over*))
* Basic Blinn-Phong lighting model (*need to port from old project*)
* Task queue for loading mesh and texture data into memory on background threads (*how do we want to do this with OCaml engine?*)

**In-progress**

* UI system

**Roadmap going forwards**

* Skeletal animation
* Terrain rendering

## Getting started

Check out these in order

<!-- * [Getting started](getting-started.md) -->
<!-- * [Project layout](project-layout.md) -->