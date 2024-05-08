/**
 * @file mem.h
 * @brief Allocators, memory tracking
 * @version 0.1
 * @date 2024-02-24
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <stddef.h>

// Inspired by https://nullprogram.com/blog/2023/09/27/
typedef struct arena {
  char* begin;
  char* curr;
  char* end;
} arena;

typedef struct arena_save {
  arena* arena;
  char* savepoint;
} arena_save;

arena arena_create(void* backing_buffer, size_t capacity);
void* arena_alloc(arena* a, size_t size);
void* arena_alloc_align(arena* a, size_t size, size_t align);
void arena_free_all(arena* a);
void arena_free_storage(arena* a);
arena_save arena_savepoint(arena* a);
void arena_rewind(arena_save savepoint);
// TODO: arena_resize