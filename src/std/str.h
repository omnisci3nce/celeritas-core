/**
 * @brief
 *
 */
#pragma once

#include "defines.h"

/**
 * @brief Fat pointer representing a UTF8 (TODO some APIs supporting utf8) encoded string
 * @note when using `printf` you must use %s.*s length, string until our own modified
         print routines are written. alternatively wrap in `cstr()` and pass to `%s`.
 */
typedef struct {
  u8 *buf;
  size_t len;
} str8;

/** @brief Compare two strings for exact equality */
bool str8_equals(str8 a, str8 b);

/**
 * @brief Compare the first `first_nchars` of each string for equality
          If either of the strings are shorter than the number only the characters up until the end
 of the shorter string will be compared.
 * @returns 0 if they are fully equal up until `first_nchars`, i.e they never differed, else it
 returns the index at which the first string differed from the second string.
*/
size_t str8_nequals(str8 a, str8 b, size_t first_nchars);

bool str8_ends_with(str8 input_str, str8 suffix);