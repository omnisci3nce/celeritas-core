// TODO: Create a SQLite-like amalgamation header to include the whole
//       API with one include and link the compiled code.

#define CEL_PLATFORM_WINDOWS

#include "defines.h"
#define c_static_inline  // remove inlined functions so we can generate bindings

#include "core.h"
#include "render.h"
#include "render_scene.h"
#include "ral.h"
#include "input.h"
#include "primitives.h"
#include "maths_types.h"
#include "maths.h"
#include "skybox.h"
#include "shadows.h"
#include "loaders.h"