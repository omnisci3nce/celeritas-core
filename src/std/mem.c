#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "log.h"
#include "mem.h"

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void*))
#endif

// --- Arena

void* arena_alloc_align(arena* a, size_t size, size_t align) {
  ptrdiff_t padding = -(uintptr_t)a->curr & (align - 1);
  ptrdiff_t available = a->end - a->curr - padding;
  // TRACE("Padding %td available %td", padding, available);
  if (available < 0 || (ptrdiff_t)size > available) {
    ERROR_EXIT("Arena ran out of memory\n");
  }
  void* p = a->curr + padding;
  a->curr += padding + size;
  return memset(p, 0, size);
}
void* arena_alloc(arena* a, size_t size) { return arena_alloc_align(a, size, DEFAULT_ALIGNMENT); }

arena arena_create(void* backing_buffer, size_t capacity) {
  return (arena){ .begin = backing_buffer,
                  .curr = backing_buffer,
                  .end = backing_buffer + (ptrdiff_t)capacity };
}

void arena_free_all(arena* a) {
  a->curr = a->begin;  // pop everything at once and reset to the start.
}

void arena_free_storage(arena* a) { free(a->begin); }

arena_save arena_savepoint(arena* a) {
  arena_save savept = { .arena = a, .savepoint = a->curr };
  return savept;
}

void arena_rewind(arena_save savepoint) { savepoint.arena->curr = savepoint.savepoint; }

// --- Pool

void_pool void_pool_create(arena* a, const char* debug_label, u64 capacity, u64 entry_size) {
  size_t memory_requirements = capacity * entry_size;
  void* backing_buf = arena_alloc(a, memory_requirements);

  assert(entry_size >= sizeof(void_pool_header));  // TODO: create my own assert with error message

  void_pool pool = { .capacity = capacity,
                     .entry_size = entry_size,
                     .count = 0,
                     .backing_buffer = backing_buf,
                     .free_list_head = NULL,
                     .debug_label = debug_label };

  void_pool_free_all(&pool);

  return pool;
}

void void_pool_free_all(void_pool* pool) {
  // set all entries to be free
  for (u64 i = 0; i < pool->capacity; i++) {
    void* ptr = &pool->backing_buffer[i * pool->entry_size];
    void_pool_header* free_node =
        (void_pool_header*)ptr;  // we reuse the actual entry itself to hold the header
    if (i == (pool->capacity - 1)) {
      // if the last one we make its next pointer NULL indicating its full
      free_node->next = NULL;
    }
    free_node->next = pool->free_list_head;
    // now the head points to this entry
    pool->free_list_head = free_node;
  }
}

void* void_pool_get(void_pool* pool, u32 raw_handle) {
  // An handle is an index into the array essentially
  void* ptr = pool->backing_buffer + (raw_handle * pool->entry_size);
  return ptr;
}

void* void_pool_alloc(void_pool* pool, u32* out_raw_handle) {
  // get the next free node
  if (pool->count == pool->capacity) {
    WARN("Pool is full!");
    return NULL;
  }
  if (pool->free_list_head == NULL) {
    ERROR("%s Pool is full (head = null)", pool->debug_label);
    return NULL;
  }
  void_pool_header* free_node = pool->free_list_head;

  // What index does this become?
  uintptr_t start = (uintptr_t)pool->backing_buffer;
  uintptr_t cur = (uintptr_t)free_node;
  TRACE("%ld %ld ", start, cur);
  assert(cur > start);
  u32 index = (u32)((cur - start) / pool->entry_size);
  /* printf("Index %d\n", index); */
  if (out_raw_handle != NULL) {
    *out_raw_handle = index;
  }

  pool->free_list_head = free_node->next;

  memset(free_node, 0, pool->entry_size);
  pool->count++;
  return (void*)free_node;
}

void void_pool_dealloc(void_pool* pool, u32 raw_handle) {
  // push free node back onto the free list
  void* ptr = void_pool_get(pool, raw_handle);
  void_pool_header* freed_node = (void_pool_header*)ptr;

  freed_node->next = pool->free_list_head;
  pool->free_list_head = freed_node;

  pool->count--;
}

u32 void_pool_insert(void_pool* pool, void* item) {
  u32 raw_handle;
  void* item_dest = void_pool_alloc(pool, &raw_handle);
  memcpy(item_dest, item, pool->entry_size);
  return raw_handle;
}