
/**
 * @file transform_hierarchy.h
 */
#pragma once
#include "transform_hierarchy.h"
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "render_types.h"

// struct transform_hierarchy {
//   transform_node root;
// };

// transform_hierarchy* transform_hierarchy_create() {
//   transform_hierarchy* tfh = malloc(sizeof(struct transform_hierarchy));

//   tfh->root = (transform_node){ .model = { ABSENT_MODEL_HANDLE },
//                                 .tf = TRANSFORM_DEFAULT,
//                                 .local_matrix_tf = mat4_ident(),
//                                 .world_matrix_tf = mat4_ident(),
//                                 .parent = NULL,
//                                 .children = { 0 },
//                                 .n_children = 0,
//                                 .tfh = tfh };
//   return tfh;
// }

// bool free_node(transform_node* node, void* _ctx_data) {
//   if (!node) return true;  // leaf node
//   if (node == &node->tfh->root) {
//     WARN("You can't free the root node!");
//     return false;
//   }

//   printf("Freed node\n");
//   free(node);
//   return true;
// }

// void transform_hierarchy_free(transform_hierarchy* tfh) {
//   transform_hierarchy_dfs(&tfh->root, free_node, false, NULL);
//   free(tfh);
// }

// transform_node* transform_hierarchy_root_node(transform_hierarchy* tfh) { return &tfh->root; }

// transform_node* transform_hierarchy_add_node(transform_node* parent, ModelHandle model,
//                                              Transform tf) {
//   if (!parent) {
//     WARN("You tried to add a node to a bad parent (NULL?)");
//     return NULL;
//   }
//   transform_node* node = malloc(sizeof(transform_node));
//   node->model = model;
//   node->tf = tf;
//   node->local_matrix_tf = mat4_ident();
//   node->world_matrix_tf = mat4_ident();
//   node->parent = parent;
//   memset(node->children, 0, sizeof(node->children));
//   node->n_children = 0;
//   node->tfh = parent->tfh;

//   // push into parent's children array
//   u32 next_index = parent->n_children;
//   if (next_index == MAX_TF_NODE_CHILDREN) {
//     ERROR("This transform hierarchy node already has MAX children. Dropping.");
//     free(node);
//   } else {
//     parent->children[next_index] = node;
//     parent->n_children++;
//   }

//   return node;
// }

// void transform_hierarchy_delete_node(transform_node* node) {
//   // delete all children
//   for (u32 i = 0; i < node->n_children; i++) {
//     transform_node* child = node->children[i];
//     transform_hierarchy_dfs(child, free_node, false, NULL);
//   }

//   if (node->parent) {
//     for (u32 i = 0; i < node->parent->n_children; i++) {
//       transform_node* child = node->parent->children[i];
//       if (child == node) {
//         node->parent->children[i] = NULL;  // HACK: this will leave behind empty slots in the
//                                            // children array of the parent. oh well.
//       }
//     }
//   }

//   free(node);
// }

// void transform_hierarchy_dfs(transform_node* start_node,
//                              bool (*visit_node)(transform_node* node, void* ctx_data),
//                              bool is_pre_order, void* ctx_data) {
//   if (!start_node) return;

//   bool continue_traversal = true;
//   if (is_pre_order) {
//     continue_traversal = visit_node(start_node, ctx_data);
//   }

//   if (continue_traversal) {
//     for (u32 i = 0; i < start_node->n_children; i++) {
//       transform_node* child = start_node->children[i];
//       transform_hierarchy_dfs(child, visit_node, is_pre_order, ctx_data);
//     }
//   }

//   if (!is_pre_order) {
//     // post-order
//     visit_node(start_node, ctx_data);
//   }
// }

// // Update matrix for the current node
// bool update_matrix(transform_node* node, void* _ctx_data) {
//   if (!node) return true;  // leaf node

//   if (node->parent && node->parent->tf.is_dirty) {
//     node->tf.is_dirty = true;
//   }

//   if (node->tf.is_dirty) {
//     // invalidates children
//     Mat4 updated_local_transform = transform_to_mat(&node->tf);
//     node->local_matrix_tf = updated_local_transform;
//     if (node->parent) {
//       Mat4 updated_world_transform =
//           mat4_mult(node->parent->world_matrix_tf, updated_local_transform);
//       node->world_matrix_tf = updated_world_transform;
//     }
//   }

//   return true;
// }

// void transform_hierarchy_propagate_transforms(transform_hierarchy* tfh) {
//   // kickoff traversal
//   transform_hierarchy_dfs(&tfh->root, update_matrix, false, NULL);
// }

// struct print_ctx {
//   Core* core;
//   u32 indentation_lvl;
// };

// bool print_node(transform_node* node, void* ctx_data) {
//   struct print_ctx* ctx = (struct print_ctx*)ctx_data;

//   if (!node) return true;
//   if (!node->parent) {
//     printf("Root Node\n");
//     ctx->indentation_lvl++;
//     return true;
//   }

//   // Grab the model
//   // FIXME
//   // model m = ctx->core->models->data[node->model.raw];
//   for (int i = 0; i < ctx->indentation_lvl; i++) {
//     printf("  ");
//   }
//   // printf("Node %s\n", m.name.buf);
//   ctx->indentation_lvl++;

//   return true;
// }

// void transform_hierarchy_debug_print(transform_node* start_node, Core* core) {
//   struct print_ctx* ctx = malloc(sizeof(struct print_ctx));
//   ctx->core = core;
//   ctx->indentation_lvl = 0;
//   transform_hierarchy_dfs(start_node, print_node, true, (void*)ctx);
//   free(ctx);
// }
