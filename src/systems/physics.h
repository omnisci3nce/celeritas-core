#pragma once

#include "maths_types.h"

// 'system' means that it gets called per frame

typedef struct physics_settings {
  f32 gravity_strength;
} physics_settings;

enum collider_type {
  cuboid_collider,
  sphere_collider,
};

/** @brief generic collider structure */
typedef struct physics_collider {
  u64 id; // ? Replace with handle?
  enum collider_type shape;
  transform transform;
  u8 layer;
  bool on_ground;
} physics_collider;

typedef struct physics_world {
  physics_settings settings;
} physics_world;

physics_world physics_init(physics_settings settings);
void physics_shutdown(physics_world* phys_world);

/** @brief perform one or more simulation steps */
void physics_system_update(physics_world* phys_world, f64 deltatime);