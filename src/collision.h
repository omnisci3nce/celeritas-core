#pragma once
#include "geometry.h"

enum ColliderType {
  CuboidCollider,
  SphereCollider,
};

/** @brief generic collider structure */
typedef struct Collider {
  u64 id;  // ? Replace with handle?
  enum ColliderType shape;
  union collider_data {
    Cuboid cuboid;
    Sphere sphere;
  } geometry;
  Transform transform;
  u8 layer;
  bool on_ground;
} Collider;
