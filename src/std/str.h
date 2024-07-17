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

#include <ctype.h>

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
} Str8;

// --- Constructors

/** @brief Take a string literal and turn it into a `str8` */
#define str8(s) \
  (Str8) { (u8*)s, ((sizeof(s) / sizeof(*(s)) - 1)) }

Str8 Str8_create(u8* buf, size_t len);

/** @brief Return a null-terminated C string cloned onto an arena */
char* Str8_to_cstr(arena* a, Str8 s);

#define cstr(a, s) (Str8_to_cstr(a, s))  // Shorthand

/** @brief Return a Str8 that references a statically allocated string.
           `string` therefore must already be null-terminated.
    @note  The backing `string` cannot be modified. */
Str8 Str8_cstr_view(char* string);

// --- Comparisons

/** @brief Compare two strings for exact equality */
bool Str8_equals(Str8 a, Str8 b);

/**
 * @brief Compare the first `first_nchars` of each string for equality
 * @details If either of the strings are shorter than the number only the characters up until the
 end of the shorter string will be compared.
 * @returns 0 if they are fully equal up until `first_nchars`, i.e they never differed, else it
            returns the index at which the first string differed from the second string.
*/
size_t Str8_nequals(Str8 a, Str8 b, size_t first_nchars);

bool Str8_ends_with(Str8 input_str, Str8 suffix);

/// --- Subviews

Str8 Str8_substr(Str8 s, u64 min, u64 max);
/** @brief Keeps only the `first_n` chars of `s` */
Str8 Str8_take(Str8 s, u64 first_n);
/** @brief Keeps only the `last_n` chars of `s` */
Str8 Str8_drop(Str8 s, u64 last_n);
/** @brief Keeps everything after the first `n` chars of `s` */
Str8 Str8_skip(Str8 s, u64 n);
/** @brief Keeps everything before the last `n` chars of `s` */
Str8 Str8_chop(Str8 s, u64 n);

Str8 Str8_concat(arena* a, Str8 left, Str8 right);

/// --- Misc

static inline bool Str8_is_null_term(Str8 a) {
  return a.buf[a.len] == 0;  // This doesn't seem safe. YOLO
}

// TODO: move or delete this and replace with handling using our internal type
static void skip_space(char* p) {
  while (isspace((unsigned char)*p)) ++p;
}
