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
#include "defines.h"

// --- Arena

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

// --- Pool

typedef struct void_pool_header void_pool_header;
struct void_pool_header {
  void_pool_header* next;
};

typedef struct void_pool {
  u64 capacity;
  u64 entry_size;
  u64 count;
  void* backing_buffer;
  void_pool_header* free_list_head;
  const char* debug_label;
} void_pool;

void_pool void_pool_create(arena* a, const char* debug_label, u64 capacity, u64 entry_size);
void void_pool_free_all(void_pool* pool);
bool void_pool_is_empty(void_pool* pool);
bool void_pool_is_full(void_pool* pool);
void* void_pool_get(void_pool* pool, u32 raw_handle);
void* void_pool_alloc(void_pool* pool, u32* out_raw_handle);
void void_pool_dealloc(void_pool* pool, u32 raw_handle);
// TODO: fn to dealloc from the pointer that was handed out

// TODO: macro that lets us specialise

/* typedef struct Name##_handle Name##_handle;      \ */
#define TYPED_POOL(T, Name)                                                          \
  typedef struct Name##_pool {                                                       \
    void_pool inner;                                                                 \
  } Name##_pool;                                                                     \
                                                                                     \
  static Name##_pool Name##_pool_create(arena* a, u64 cap, u64 entry_size) {         \
    void_pool p = void_pool_create(a, "\"" #Name "\"", cap, entry_size);             \
    return (Name##_pool){ .inner = p };                                              \
  }                                                                                  \
  static inline T* Name##_pool_get(Name##_pool* pool, Name##_handle handle) {        \
    return (T*)void_pool_get(&pool->inner, handle.raw);                              \
  }                                                                                  \
  static inline T* Name##_pool_alloc(Name##_pool* pool, Name##_handle* out_handle) { \
    return (T*)void_pool_alloc(&pool->inner, &out_handle->raw);                      \
  }                                                                                  \
  static inline void Name##_pool_dealloc(Name##_pool* pool, Name##_handle handle) {  \
    void_pool_dealloc(&pool->inner, handle.raw);                                     \
  }\
