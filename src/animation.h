#pragma once

#include "darray.h"
#include "defines.h"
#include "maths_types.h"

KITC_DECL_TYPED_ARRAY(f32)

typedef enum interpolation { INTERPOLATION_LINEAR, INTERPOLATION_COUNT } interpolation;

typedef enum keyframe_kind {
  KEYFRAME_ROTATION,
  KEYFRAME_TRANSLATION,
  KEYFRAME_SCALE,
  KEYFRAME_WEIGHTS,
} keyframe_kind;

typedef union keyframe {
  quat rotation;
  vec3 translation;
  vec3 scale;
  float* weights;
} keyframe;

typedef struct keyframes {
  keyframe_kind kind;
  keyframe* values;
  size_t count;
} keyframes;

typedef struct animation_spline {
  f32_darray timestamps;
  keyframes values;
  interpolation interpolation;
} animation_spline;

typedef struct animation_sampler {
  int current_index;
  animation_spline animation;
} animation_sampler;

/** @brief Sample an animation at a given time `t` */
keyframe animation_sample(animation_sampler sampler, f32 t);

typedef struct animation_clip {
  // A clip contains one or more animation curves
  // for now I think we can just enumerate all of the properties (assuming *only* one per type is in
  // a clip) NULL = this property is not animated in this clip
  animation_sampler* rotation;
  animation_sampler* translation;
  animation_sampler* scale;
  animation_sampler* weights;
} animation_clip;