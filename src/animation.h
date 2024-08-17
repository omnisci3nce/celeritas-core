#pragma once

#include "cgltf.h"
#include "darray.h"
#include "defines.h"
#include "maths_types.h"
#include "mem.h"
#include "ral_types.h"

typedef enum Interpolation {
  INTERPOLATION_STEP,
  INTERPOLATION_LINEAR,
  INTERPOLATION_CUBIC, /** @brief Cubic spline interpolation */
  INTERPOLATION_COUNT
} Interpolation;

typedef enum KeyframeKind {
  KEYFRAME_ROTATION,
  KEYFRAME_TRANSLATION,
  KEYFRAME_SCALE,
  KEYFRAME_WEIGHTS,
} KeyframeKind;

static const char* keyframe_kind_strings[4] = { "ROTATION", "TRANSLATION", "SCALE", "WEIGHTS"};

typedef union Keyframe {
  Quat rotation;
  Vec3 translation;
  Vec3 scale;
  f32 weights[4];
} Keyframe;

typedef struct Keyframes {
  KeyframeKind kind;
  Keyframe* values;
  size_t count;
} Keyframes;

typedef struct Joint {
  // used instead of pointers later to update correct joints
  size_t node_idx;
  ssize_t parent; // parent bone. -1 means its the root
  size_t children[8]; // children bones, upto 8
  u8 children_count;
  char* debug_label;  // optional
  Mat4 inverse_bind_matrix;
  Mat4 local_transform;
  /** @brief holds position, rotation, and scale that will be written to by animation
  samplers every tick of the animation system. */
  Transform transform_components;
} Joint;
#ifndef TYPED_JOINT_ARRAY
KITC_DECL_TYPED_ARRAY(Joint);
#define TYPED_JOINT_ARRAY
#endif

typedef u32 JointIdx;

typedef struct Armature {
  char* label;
  Joint_darray* joints;
} Armature;

// NOTE: I think we will need to topologically sort the joints to store them in array if we want to
// do linear array traversal
//       when calculating transforms.

typedef struct AnimationSpline {
  f32* timestamps;
  size_t n_timestamps;
  Keyframes values;
  Interpolation interpolation;
} AnimationSpline;

// combines a sampler and a channel in gltf
typedef struct AnimationSampler {
  int current_index;
  f32 min;
  f32 max;
  AnimationSpline animation;
  u32 target_joint_idx; // index into the array of joints in an armature
} AnimationSampler;
#ifndef TYPED_ANIM_SAMPLER_ARRAY
KITC_DECL_TYPED_ARRAY(AnimationSampler);
#define TYPED_ANIM_SAMPLER_ARRAY
#endif

/** @brief Sample an animation at a given time `t` returning an interpolated keyframe */
PUB Keyframe Animation_Sample(AnimationSampler* sampler, f32 t);

/** @brief A clip contains one or more animation curves. */
typedef struct AnimationClip {
  const char* clip_name;
  bool is_playing;
  f32 time;
  AnimationSampler_darray* channels;
} AnimationClip;
#ifndef TYPED_ANIM_CLIP_ARRAY
KITC_DECL_TYPED_ARRAY(AnimationClip);
#define TYPED_ANIM_CLIP_ARRAY
#endif

// typedef struct SkinnedAnimation {
//   Mat4* joint_matrices;
//   size_t n_joints;
// } SkinnedAnimation;

PUB void Animation_Play(AnimationClip* clip);

void Animation_Tick(AnimationClip* clip, Armature* armature, f32 delta_time);

void Animation_VisualiseJoints(Armature* armature);

#define MAX_BONES 100

typedef struct AnimDataUniform {
    Mat4 bone_matrices[MAX_BONES];
} AnimDataUniform;
ShaderDataLayout AnimData_GetLayout(void* data);

// Animation Targets:
// - Mesh
// - Joint
