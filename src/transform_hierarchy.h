/**
 * @file transform_hierarchy.h
 */
#pragma once

#include "maths_types.h"
#include "ral.h"
#include "render_types.h"

#define MAX_TF_NODE_CHILDREN \
  32 /** TEMP: Make it simpler to manage children in `transform_node`s　*/

typedef struct TransformHierarchy TransformHierarchy;

struct Transform_Node {
  ModelHandle model; /** A handle back to what model this node represents */
  Transform tf;
  Mat4 local_matrix_tf; /** cached local affine transform */
  Mat4 world_matrix_tf; /** cached world-space affine transform */

  struct transform_node* parent;
  struct transform_node* children[MAX_TF_NODE_CHILDREN];
  u32 n_children;
  struct transform_hierarchy* tfh;
};
typedef struct Transform_Node Transform_Node;
typedef struct Transform_Node TF_Node;

// --- Lifecycle

/** @brief Allocates and returns an empty transform hierarchy with a root node */
TransformHierarchy* TransformHierarchy_Create();

/**
 * @brief recursively frees all the children and then finally itself
 * @note in the future we can use an object pool for the nodes
 */
void transform_hierarchy_free(transform_hierarchy* tfh);

// --- Main usecase

/** @brief Updates matrices of any invalidated nodes based on the `is_dirty` flag inside `transform`
 */
void transform_hierarchy_propagate_transforms(transform_hierarchy* tfh);

// --- Queries

/** @brief Get a pointer to the root node */
Transform_Node* TransformHierarchy_RootNode(TransformHierarchy* tfh);

// --- Mutations
Transform_Node* TransformHierarchy_AddNode(transform_node* parent, ModelHandle model,
                                             Transform tf);
void transform_hierarchy_delete_node(transform_node* node);

// --- Traversal

/**
 * @brief Perform a depth-first search traversal starting from `start_node`.
 * @param start_node The starting node of the traversal.
 * @param visit_node The function to call for each node visited. The callback should return false to
 stop the traversal early.
 * @param is_pre_order Indicates whether to do pre-order or post-order traversal i.e. when to call
 the `visit_node` function.
 * @param ctx_data An optional pointer to data that is be passed on each call to `visit_node`. Can
 be used to carry additional information or context.
 *
 * @note The main use-cases are:
            1. traversing the whole tree to update cached 4x4 affine transform matrices (post-order)
            2. freeing child nodes after deleting a node in the tree (post-order)
            3. debug pretty printing the whole tree (post-order)
 */
void transform_hierarchy_dfs(transform_node* start_node,
                             bool (*visit_node)(transform_node* node, void* ctx_data),
                             bool is_pre_order, void* ctx_data);

struct Core;
void transform_hierarchy_debug_print(transform_node* start_node, struct Core* core);
