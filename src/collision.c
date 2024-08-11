#include "immdraw.h"
#include "maths.h"
#include "maths_types.h"
#include "physics.h"

PUB void Debug_DrawOBB(OBB obb) {
  Transform t = transform_create(obb.center, obb.rotation, vec3_sub(obb.bbox.max, obb.bbox.min));
  Immdraw_Cuboid(t, vec4(0.0, 0.8, 0.1, 1.0), true);
}
