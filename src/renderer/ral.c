#include "ral.h"

/* typedef struct foo { */
/*   u32 a; */
/*   f32 b; */
/*   char c; */
/* } foo; */

/* TYPED_POOL(gpu_buffer, buffer); */

/* typedef struct buffer_handle { */
/*   u32 raw; */
/* } buffer_handle; */

/* typedef struct gpu_buffer gpu_buffer; */
TYPED_POOL(gpu_buffer, buffer);
TYPED_POOL(gpu_texture, texture);

struct resource_pools {
  buffer_pool buffers;
  texture_pool textures;
};
