/**
 * @file defines.h
 * @author your name (you@domain.com)
 * @brief 
 * @date 2024-02-24
 * @copyright Copyright (c) 2024
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

_Static_assert(sizeof(bool) == 1, "type bool should be 1 byte");

_Static_assert(sizeof(u8) == 1, "type u8 should be 1 byte");
_Static_assert(sizeof(u16) == 2, "type u16 should be 2 byte");
_Static_assert(sizeof(u32) == 4, "type u32 should be 4 byte");
_Static_assert(sizeof(u64) == 8, "type u64 should be 8 byte");

_Static_assert(sizeof(i8) == 1, "type i8 should be 1 byte");
_Static_assert(sizeof(i16) == 2, "type i16 should be 2 byte");
_Static_assert(sizeof(i32) == 4, "type i32 should be 4 byte");
_Static_assert(sizeof(i64) == 8, "type i64 should be 8 byte");

_Static_assert(sizeof(f32) == 4, "type f32 should be 4 bytes");
_Static_assert(sizeof(f64) == 8, "type f64 should be 8 bytes");

_Static_assert(sizeof(ptrdiff_t) == 8, "");

#define alignof(x) _Alignof(x)