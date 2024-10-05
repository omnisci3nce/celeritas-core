#pragma once
#include <stdbool.h>

// Defines "_sarray" types

#define TYPED_STACK_ARRAY(T, Name, Len) \
    typedef struct Name##_sarray { \
        T items[ Len ]; \
        size_t len; \
    } Name##_sarray; \
    Name##_sarray Name##_sarray_create() { \
        Name##_sarray arr = { .len = 0 }; \
        return arr; \
    } \
    bool Name##_sarray_push(Name##_sarray* arr, T item) { \
        if (arr->len == Len) { return false; }\
        arr->items[arr->len++] = item;\
        return true;\
    }
