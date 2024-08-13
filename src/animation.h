#pragma once

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
  char* debug_label;  // optional
  Mat4 inverse_bind_matrix;
  Mat4 local_transform;
  Transform transform_components;
} Joint;
#ifndef TYPED_JOINT_ARRAY
KITC_DECL_TYPED_ARRAY(Joint);
#define TYPED_JOINT_ARRAY
#endif

typedef u32 JointIdx;

typedef struct Armature {
  char* label;
  arena arena;
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

typedef struct AnimationSampler {
  int current_index;
  f32 min;
  f32 max;
  AnimationSpline animation;
} AnimationSampler;

/** @brief Sample an animation at a given time `t` returning an interpolated keyframe */
PUB Keyframe Animation_Sample(AnimationSampler* sampler, f32 t);

typedef struct AnimationClip {
  // A clip contains one or more animation curves.
  // For now I think we can just enumerate all of the properties (assuming *only* one per type is in
  // a clip) and NULL = the property is not animated in this clip
  // NOTE: when I call 'property' is typically referred to as a 'channel' by GLTF
  AnimationSampler* rotation;
  AnimationSampler* translation;
  AnimationSampler* scale;
  AnimationSampler* weights;
} AnimationClip;
#ifndef TYPED_ANIM_CLIP_ARRAY
KITC_DECL_TYPED_ARRAY(AnimationClip);
#define TYPED_ANIM_CLIP_ARRAY
#endif

typedef struct SkinnedAnimation {
  Mat4* joint_matrices;
  size_t n_joints;
} SkinnedAnimation;

PUB void Animation_Play(AnimationClip* clip);

void Animation_VisualiseJoints(Armature* armature);

typedef struct AnimDataUniform {
    Mat4 bone_matrices[4];
} AnimDataUniform;
ShaderDataLayout AnimData_GetLayout(void* data);
