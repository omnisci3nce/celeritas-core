/**
 * @file collision.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include "geometry.h"

 enum collider_type {
  cuboid_collider,
  sphere_collider,
};

/** @brief generic collider structure */
typedef struct physics_collider {
  u64 id;  // ? Replace with handle?
  enum collider_type shape;
  union collider_data {
    cuboid cuboid;
    sphere sphere;
  } geometry;
  transform transform;
  u8 layer;
  bool on_ground;
} physics_collider;