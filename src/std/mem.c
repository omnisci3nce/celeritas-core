#include "mem.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "log.h"

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void*))
#endif

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