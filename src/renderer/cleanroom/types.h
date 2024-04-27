#pragma once
#include "darray.h"
#include "defines.h"
#include "maths_types.h"
#include "str.h"
#include "render_types.h"

// typedef struct transform_hierarchy {
// } transform_hierarchy;


/*
 - render_types.h
 - ral_types.h
 - ral.h
 - render.h ?
*/

/* render_types */
typedef struct model pbr_material;
typedef struct model bp_material;  // blinn-phong


// ? How to tie together materials and shaders

// Three registers
// 1. low level graphics api calls "ral"
// 2. higher level render calls
// 3. simplified immediate mode API

// 3 - you don't need to know how the renderer works at all
// 2 - you need to know how the overall renderer is designed
// 1 - you need to understand graphics API specifics

/* ral.h */

// command buffer gubbins

/* --- Backends */

// struct vulkan_backend {
//   gpu_pipeline static_opaque_pipeline;
//   gpu_pipeline skinned_opaque_pipeline;
// };

/* --- Renderer layer */
/* render.h */


// Drawing

// void draw_mesh(gpu_cmd_encoder* encoder, mesh* mesh) {
//   encode_set_vertex_buffer(encoder, mesh->vertex_buffer);
//   encode_set_index_buffer(encoder, mesh->index_buffer);
//   encode_draw_indexed(encoder, mesh->index_count)
//   // vkCmdDrawIndexed
// }

// void draw_scene(arena* frame, model_darray* models, renderer* ren, camera* camera,
//                 transform_hierarchy* tfh, scene* scene) {
//                   // set the pipeline first
//                   encode_set_pipeline()
//                   // in open this sets the shader
//                   // in vulkan it sets the whole pipeline

// }