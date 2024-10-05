/**
 * @file hashset.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "defines.h"

typedef struct hashset hashset;

/** @brief Describes a function that will take a pointer to a datatype (e.g. a u64 or a struct)
           and return a hashed key. */
typedef uint64_t (*hash_item)(void* item);

void hashset_init(hashset* set, hash_item hash_func, size_t initial_capacity);
// TODO: void hashset_from_iterator();
bool hashset_insert(hashset* set, void* item, uint64_t* out_key);
void hashset_batch_insert(hashset* set, void* items, u64 item_count);
bool hashset_contains(hashset* set, void* item);
bool hashset_remove_item(hashset* set, void* item);
bool hashset_remove_key(hashset* set, uint64_t key);
void hashset_merge(hashset* set_a, hashset* set_b);
hashset* hashset_merge_cloned(hashset* set_a, hashset* set_b);
void hashset_free(hashset* set);