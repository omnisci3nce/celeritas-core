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

arena arena_create(void* backing_buffer, size_t capacity);
void* arena_alloc(arena* a, size_t size);
void* arena_alloc_align(arena* a, size_t size, size_t align);
void arena_free_all(arena* a);
// TODO: arena_resize