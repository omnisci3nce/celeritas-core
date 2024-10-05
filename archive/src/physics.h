#pragma once

#include "geometry.h"
#include "maths_types.h"

// 'system' means that it gets called per frame

typedef struct physics_settings {
  f32 gravity_strength;
} physics_settings;

// What else do I need?
// intersection methods

typedef struct physics_world {
  physics_settings settings;
} physics_world;

physics_world physics_init(physics_settings settings);
void physics_shutdown(physics_world* phys_world);

/** @brief perform one or more simulation steps */
void physics_system_update(physics_world* phys_world, f64 deltatime);

// enum ColliderType {
//   CuboidCollider,
//   SphereCollider,
// };

/** @brief Oriented Bounding Box */
typedef struct OBB {
  Vec3 center;
  Bbox_3D bbox;
  Quat rotation;
} OBB;

PUB void Debug_DrawOBB(OBB obb);

/** @brief generic collider structure */
typedef struct Collider {
  u64 id;     // ? Replace with handle?
  OBB shape;  // NOTE: We're only supporting the one collider type for now
  bool on_ground;
} Collider;
