#include "animation.h"
#include "maths.h"
#include "log.h"

keyframe animation_sample(animation_sampler *sampler, f32 t) {
  size_t previous_index = 0;
  f32 previous_time = 0.0;
  // look forwards
  DEBUG("%d\n", sampler->animation.values.kind);
  TRACE("Here %d", sampler->animation.n_timestamps);
  for (u32 i = 1; i < sampler->animation.n_timestamps; i++) {
    f32 current_time = sampler->animation.timestamps[i];
    if (current_time > t) {
      break;
    }
    previous_time = current_time;
    previous_index = i;
    
  }

  size_t next_index = (previous_index + 1) % sampler->animation.n_timestamps;
  f32 next_time = sampler->animation.timestamps[next_index];
  printf("%d %f %d %f\n", previous_index, previous_time, next_index, next_time);

  keyframe prev_value = sampler->animation.values.values[previous_index];
  keyframe next_value = sampler->animation.values.values[next_index];

  printf("%d %d\n", previous_index, next_index);
   
      f32 time_diff = sampler->animation.timestamps[next_index] - sampler->animation.timestamps[previous_index
      ];
      f32 percent = (t - sampler->animation.timestamps[next_index]) / time_diff;

      quat interpolated_rot = quat_slerp(sampler->animation.values.values[previous_index].rotation,
                                         sampler->animation.values.values[next_index].rotation, percent);

     return  (keyframe){ .rotation = interpolated_rot };
}