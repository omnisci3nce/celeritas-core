/**
 * @file darray.h
 * @brief Typed dynamic array
 * @copyright Copyright (c) 2023
 */
// COPIED FROM KITC WITH SOME MINOR ADJUSTMENTS

/* TODO:
     - a 'find' function that takes a predicate (maybe wrap with a macro so we dont have to define a new function?)
*/

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

#define KITC_DECL_TYPED_ARRAY(T) DECL_TYPED_ARRAY(T, T)

#define DECL_TYPED_ARRAY(T, Type)                                                         \
  typedef typed_array(T) Type##_darray;                                                   \
  typedef typed_array_iterator(Type) Type##_darray_iter;                                  \
                                                                                          \
  /* Create a new one growable array */                                                   \
  PREFIX Type##_darray *Type##_darray_new(size_t starting_capacity) {                     \
    Type##_darray *d;                                                                     \
    T *data;                                                                              \
    d = malloc(sizeof(Type##_darray));                                                    \
    data = malloc(starting_capacity * sizeof(T));                                         \
                                                                                          \
    d->len = 0;                                                                           \
    d->capacity = starting_capacity;                                                      \
    d->data = data;                                                                       \
                                                                                          \
    return d;                                                                             \
  }                                                                                       \
                                                                                          \
  PREFIX void Type##_darray_free(Type##_darray *d) {                                      \
    if (d != NULL) {                                                                      \
      free(d->data);                                                                      \
      free(d);                                                                            \
    }                                                                                     \
  }                                                                                       \
                                                                                          \
  PREFIX T *Type##_darray_resize(Type##_darray *d, size_t capacity) {                     \
    /* resize the internal data block */                                                  \
    T *new_data = realloc(d->data, sizeof(T) * capacity);                                 \
    /* TODO: handle OOM error */                                                          \
                                                                                          \
    d->capacity = capacity;                                                               \
    d->data = new_data;                                                                   \
    return new_data;                                                                      \
  }                                                                                       \
                                                                                          \
  PREFIX void Type##_darray_push(Type##_darray *d, T value) {                             \
    if (d->len >= d->capacity) {                                                          \
      size_t new_capacity =                                                               \
          d->capacity > 0 ? d->capacity * DARRAY_RESIZE_FACTOR : DARRAY_DEFAULT_CAPACITY; \
      T *resized = Type##_darray_resize(d, new_capacity);                                 \
    }                                                                                     \
                                                                                          \
    d->data[d->len] = value;                                                              \
    d->len += 1;                                                                          \
  }                                                                                       \
                                                                                          \
  PREFIX void Type##_darray_push_copy(Type##_darray *d, const T *value) {                 \
    if (d->len >= d->capacity) {                                                          \
      size_t new_capacity =                                                               \
          d->capacity > 0 ? d->capacity * DARRAY_RESIZE_FACTOR : DARRAY_DEFAULT_CAPACITY; \
      T *resized = Type##_darray_resize(d, new_capacity);                                 \
    }                                                                                     \
                                                                                          \
    T *place = d->data + d->len;                                                          \
    d->len += 1;                                                                          \
    memcpy(place, value, sizeof(T));                                                      \
  }                                                                                       \
                                                                                          \
  PREFIX void Type##_darray_pop(Type##_darray *d, T *dest) {                              \
    T *item = d->data + (d->len - 1);                                                     \
    d->len -= 1;                                                                          \
    memcpy(dest, item, sizeof(T));                                                        \
  }                                                                                       \
                                                                                          \
  PREFIX void Type##_darray_ins(Type##_darray *d, const T *value, size_t index) {         \
    /* check if requires resize */                                                        \
    if (d->len + 1 > d->capacity) {                                                       \
      size_t new_capacity =                                                               \
          d->capacity > 0 ? d->capacity * DARRAY_RESIZE_FACTOR : DARRAY_DEFAULT_CAPACITY; \
      T *resized = Type##_darray_resize(d, new_capacity);                                 \
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
  PREFIX void Type##_darray_clear(Type##_darray *d) {                                     \
    d->len = 0;                                                                           \
    memset(d->data, 0, d->capacity * sizeof(T));                                          \
  }                                                                                       \
                                                                                          \
  PREFIX size_t Type##_darray_len(Type##_darray *d) { return d->len; }                    \
                                                                                          \
  PREFIX void Type##_darray_print(Type##_darray *d) {                                     \
    printf("len: %zu ", d->len);                                                          \
    printf("capacity: %zu\n", d->capacity);                                               \
    for (int i = 0; i < d->len; i++) {                                                    \
      printf("Index %d holds value %d\n", i, d->data[i]);                                 \
    }                                                                                     \
  }                                                                                       \
                                                                                          \
  PREFIX Type##_darray_iter Type##_darray_iter_new(Type##_darray *d) {                    \
    Type##_darray_iter iterator;                                                          \
    iterator.array = d;                                                                   \
    iterator.current_idx = 0;                                                             \
    return iterator;                                                                      \
  }                                                                                       \
                                                                                          \
  PREFIX void *Type##_darray_iter_next(Type##_darray_iter *iterator) {                    \
    if (iterator->current_idx < iterator->array->len) {                                   \
      return &iterator->array->data[iterator->current_idx++];                             \
    } else {                                                                              \
      return NULL;                                                                        \
    }                                                                                     \
  }

#endif  // KITC_TYPED_ARRAY_H
