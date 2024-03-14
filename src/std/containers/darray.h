/**
 * @file darray.h
 * @brief Typed dynamic array
 * @copyright Copyright (c) 2023
 */
// COPIED FROM KITC WITH SOME MINOR ADJUSTMENTS

#ifndef KITC_TYPED_ARRAY_H
#define KITC_TYPED_ARRAY_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DARRAY_DEFAULT_CAPACITY 64
#define DARRAY_RESIZE_FACTOR 3

/** @brief create a new darray type and functions with type `N` */
#define typed_array(T)                                \
  struct {                                            \
    /* @brief current number of items in the array */ \
    size_t len;                                       \
    size_t capacity;                                  \
    T *data;                                          \
  }

#define typed_array_iterator(T) \
  struct {                      \
    T##_darray *array;          \
    size_t current_idx;         \
  }

#define PREFIX static

/* if (arena != NULL) {\ */
/* d = arena_alloc(arena, sizeof(T##_darray));\ */
/* data = arena_alloc(arena, starting_capacity * sizeof(T));\ */
/* } else {\ */
/* }\ */

#define KITC_DECL_TYPED_ARRAY(T)                                                          \
  typedef typed_array(T) T##_darray;                                                      \
  typedef typed_array_iterator(T) T##_darray_iter;                                        \
                                                                                          \
  /* Create a new one growable array */                                                   \
  PREFIX T##_darray *T##_darray_new(size_t starting_capacity) {                           \
    T##_darray *d;                                                                        \
    T *data;                                                                              \
    d = malloc(sizeof(T##_darray));                                                       \
    data = malloc(starting_capacity * sizeof(T));                                         \
                                                                                          \
    d->len = 0;                                                                           \
    d->capacity = starting_capacity;                                                      \
    d->data = data;                                                                       \
                                                                                          \
    return d;                                                                             \
  }                                                                                       \
                                                                                          \
  PREFIX void T##_darray_free(T##_darray *d) {                                            \
    if (d != NULL) {                                                                      \
      free(d->data);                                                                      \
      free(d);                                                                            \
    }                                                                                     \
  }                                                                                       \
                                                                                          \
  PREFIX T *T##_darray_resize(T##_darray *d, size_t capacity) {                           \
    /* resize the internal data block */                                                  \
    T *new_data = realloc(d->data, sizeof(T) * capacity);                                 \
    /* TODO: handle OOM error */                                                          \
                                                                                          \
    d->capacity = capacity;                                                               \
    d->data = new_data;                                                                   \
    return new_data;                                                                      \
  }                                                                                       \
                                                                                          \
  PREFIX void T##_darray_push(T##_darray *d, T value) {                                   \
    if (d->len >= d->capacity) {                                                          \
      size_t new_capacity =                                                               \
          d->capacity > 0 ? d->capacity * DARRAY_RESIZE_FACTOR : DARRAY_DEFAULT_CAPACITY; \
      T *resized = T##_darray_resize(d, new_capacity);                                    \
    }                                                                                     \
                                                                                          \
    d->data[d->len] = value;                                                              \
    d->len += 1;                                                                          \
  }                                                                                       \
                                                                                          \
  PREFIX void T##_darray_push_copy(T##_darray *d, const T *value) {                       \
    if (d->len >= d->capacity) {                                                          \
      size_t new_capacity =                                                               \
          d->capacity > 0 ? d->capacity * DARRAY_RESIZE_FACTOR : DARRAY_DEFAULT_CAPACITY; \
      T *resized = T##_darray_resize(d, new_capacity);                                    \
    }                                                                                     \
                                                                                          \
    T *place = d->data + d->len;                                                          \
    d->len += 1;                                                                          \
    memcpy(place, value, sizeof(T));                                                      \
  }                                                                                       \
                                                                                          \
  PREFIX void T##_darray_pop(T##_darray *d, T *dest) {                                    \
    T *item = d->data + (d->len - 1);                                                     \
    d->len -= 1;                                                                          \
    memcpy(dest, item, sizeof(T));                                                        \
  }                                                                                       \
                                                                                          \
  PREFIX void T##_darray_ins(T##_darray *d, const T *value, size_t index) {               \
    /* check if requires resize */                                                        \
    if (d->len + 1 > d->capacity) {                                                       \
      size_t new_capacity =                                                               \
          d->capacity > 0 ? d->capacity * DARRAY_RESIZE_FACTOR : DARRAY_DEFAULT_CAPACITY; \
      T *resized = T##_darray_resize(d, new_capacity);                                    \
    }                                                                                     \
                                                                                          \
    /* shift existing data after index */                                                 \
    T *insert_dest = d->data + index;                                                     \
    T *shift_dest = insert_dest + 1;                                                      \
                                                                                          \
    int num_items = d->len - index;                                                       \
                                                                                          \
    d->len += 1;                                                                          \
    memcpy(shift_dest, insert_dest, num_items * sizeof(T));                               \
    memcpy(insert_dest, value, sizeof(T));                                                \
  }                                                                                       \
                                                                                          \
  PREFIX void T##_darray_clear(T##_darray *d) {                                           \
    d->len = 0;                                                                           \
    memset(d->data, 0, d->capacity * sizeof(T));                                          \
  }                                                                                       \
                                                                                          \
  PREFIX size_t T##_darray_len(T##_darray *d) { return d->len; }                          \
                                                                                          \
  PREFIX void T##_darray_print(T##_darray *d) {                                           \
    printf("len: %zu ", d->len);                                                          \
    printf("capacity: %zu\n", d->capacity);                                               \
    for (int i = 0; i < d->len; i++) {                                                    \
      printf("Index %d holds value %d\n", i, d->data[i]);                                 \
    }                                                                                     \
  }                                                                                       \
                                                                                          \
  PREFIX T##_darray_iter T##_darray_iter_new(T##_darray *d) {                             \
    T##_darray_iter iterator;                                                             \
    iterator.array = d;                                                                   \
    iterator.current_idx = 0;                                                             \
    return iterator;                                                                      \
  }                                                                                       \
                                                                                          \
  PREFIX void *T##_darray_iter_next(T##_darray_iter *iterator) {                          \
    if (iterator->current_idx < iterator->array->len) {                                   \
      return &iterator->array->data[iterator->current_idx++];                             \
    } else {                                                                              \
      return NULL;                                                                        \
    }                                                                                     \
  }

#endif  // KITC_TYPED_ARRAY_H
