/**
 * @file hashmap.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-27
 *
 * @copyright Copyright (c) 2024
 *
 */


typedef struct hashmap hashmap;

/*
Example usage
-------------
init hashmap
insert (string, material)
get (string) -> material_opt or material* ?
 
*/

void hashmap_init(hashmap* map);

// ...

void hashmap_free(hashmap* map);