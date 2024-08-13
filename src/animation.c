#include "animation.h"
#include "immdraw.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "ral_types.h"
#include "transform_hierarchy.h"

Keyframe Animation_Sample(AnimationSampler* sampler, f32 t) {
  size_t previous_index = 0;
  f32 previous_time = 0.0;
  // look forwards
  DEBUG("%d\n", keyframe_kind_strings[sampler->animation.values.kind]);
  TRACE("Total timestamps %d", sampler->animation.n_timestamps);
  for (u32 i = 0; i < sampler->animation.n_timestamps; i++) {
    f32 current_time = sampler->animation.timestamps[i];
    if (current_time > t) {
      break;
    }
    previous_time = sampler->animation.timestamps[i];
    previous_index = i;
  }

  size_t next_index = (previous_index + 1) % sampler->animation.n_timestamps;
  f32 next_time = sampler->animation.timestamps[next_index];
  printf("%d %f %d %f\n", previous_index, previous_time, next_index, next_time);

  Keyframe prev_value = sampler->animation.values.values[previous_index];
  Keyframe next_value = sampler->animation.values.values[next_index];

  printf("%d %d\n", previous_index, next_index);

  f32 time_diff =
      sampler->animation.timestamps[next_index] - sampler->animation.timestamps[previous_index];
  f32 percent = (t - previous_time) / time_diff;

  switch (sampler->animation.values.kind) {
    case KEYFRAME_ROTATION:
      return (Keyframe){ .rotation = quat_slerp(
                             sampler->animation.values.values[previous_index].rotation,
                             sampler->animation.values.values[next_index].rotation, percent) };
    case KEYFRAME_TRANSLATION:
    case KEYFRAME_SCALE:
    case KEYFRAME_WEIGHTS:
      WARN("TODO: other keyframe kind interpolation");
      return prev_value;
  }
}

void Animation_VisualiseJoints(Armature* armature) {
  for (int j = 0; j < armature->joints->len; j++) {
    Joint joint = armature->joints->data[j];
    Transform tf = joint.transform_components;
    tf.scale = vec3(0.05, 0.05, 0.05);
    Immdraw_Sphere(tf, vec4(0, 1, 1, 1), true);
  }
}

ShaderDataLayout AnimData_GetLayout(void* data) {
  AnimDataUniform* d = data;
  bool has_data = data != NULL;

  ShaderBinding b1 = { .label = "AnimData",
                       .kind = BINDING_BYTES,
                       .data.bytes.size = sizeof(AnimDataUniform) };

  if (has_data) {
    b1.data.bytes.data = d;
  }
  return (ShaderDataLayout){ .bindings = { b1 }, .binding_count = 1 };
}
