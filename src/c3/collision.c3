module collision;
import std::math;

struct Cuboid {
  Vec3f half_extents;
}

struct Transform {
  Vec3f position;
  Quaternion rotation;
  Vec3f scale;
}

// A cuboid physics collider
struct Collider {
  ulong id;
  Cuboid bounding_box;
  Transform transform;
  uint layer;
  bool on_ground;
}
