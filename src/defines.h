/**
 * @file defines.h
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

#define thread_local _Thread_local
#define core_noreturn _Noreturn

// Wrap a u32 to make a type-safe "handle" or ID
#define CORE_DEFINE_HANDLE(name) \
  typedef struct name name;      \
  struct name {                  \
    u32 raw;                     \
  }

/*
Possible platform defines:
#define CEL_PLATFORM_LINUX 1
#define CEL_PLATFORM_WINDOWS 1
#define CEL_PLATFORM_MAC 1
#define CEL_PLATFORM_HEADLESS 1
*/

/*
Renderer backend defines:
#define CEL_REND_BACKEND_OPENGL 1
#define CEL_REND_BACKEND_VULKAN 1
#define CEL_REND_BACKEND_METAL 1
*/

// Platform will inform renderer backend (unless user overrides)
#if defined(CEL_PLATFORM_LINUX) || defined(CEL_PLATFORM_WINDOWS)
#define CEL_REND_BACKEND_OPENGL 1
// #define CEL_REND_BACKEND_VULKAN 1
#endif

#if defined(CEL_PLATFORM_MAC)
#define CEL_REND_BACKEND_METAL 1
// #define CEL_REND_BACKEND_OPENGL 1
#endif