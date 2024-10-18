/**
 * @file scene.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <celeritas.h>

// Retained mode scene tree that handles performant transform propagation, and allows systems, or other languages via bindings, to
// manipulate rendering/scene data without *owning* said data.

typedef struct scene_tree_node {
  const char* label;
} scene_tree_node;

DEFINE_HANDLE(scene_node_handle);
TYPED_POOL(scene_tree_node, scene_node);

typedef struct render_scene_tree {
} render_scene_tree;

// What kind of operations and mutations can we perform on the tree?
