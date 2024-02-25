/**
 * @file str.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "defines.h"
#include "mem.h"

/**
 * @brief Fat pointer representing a UTF8 (TODO some APIs supporting utf8) encoded string
 * @note when using `printf` you must use %s.*s length, string until our own modified
         print routines are written. alternatively wrap in `cstr()` and pass to `%s`.
 */
typedef struct {
  u8* buf;
  size_t len;
} str8;

// --- Constructors

/** @brief Take a string literal and turn it into a `str8` */
#define str8lit(s) \
  (str8) { (u8*)s, ((sizeof(s) / sizeof(*(s)) - 1)) }

str8 str8_create(u8* buf, size_t len);

/** @brief Return a null-terminated C string cloned onto an arena */
char* str8_to_cstr(arena* a, str8 s);

#define cstr(a, s) (str8_to_cstr(a, s))  // Shorthand

// --- Comparisons

/** @brief Compare two strings for exact equality */
bool str8_equals(str8 a, str8 b);

/**
 * @brief Compare the first `first_nchars` of each string for equality
 * @details If either of the strings are shorter than the number only the characters up until the
 end of the shorter string will be compared.
 * @returns 0 if they are fully equal up until `first_nchars`, i.e they never differed, else it
            returns the index at which the first string differed from the second string.
*/
size_t str8_nequals(str8 a, str8 b, size_t first_nchars);

bool str8_ends_with(str8 input_str, str8 suffix);

/// --- Subviews

str8 str8_substr(str8 s, u64 min, u64 max);
/** @brief Keeps only the `first_n` chars of `s` */
str8 str8_take(str8 s, u64 first_n);
/** @brief Keeps only the `last_n` chars of `s` */
str8 str8_drop(str8 s, u64 last_n);
/** @brief Keeps everything after the first `n` chars of `s` */
str8 str8_skip(str8 s, u64 n);
/** @brief Keeps everything before the last `n` chars of `s` */
str8 str8_chop(str8 s, u64 n);

str8 str8_concat(arena* a, str8 left, str8 right);

/// --- Misc

static inline bool str8_is_null_term(str8 a) {
  return a.buf[a.len] == 0;  // This doesn't seem safe. YOLO
}