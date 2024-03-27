/**
 * @file file.h
 * @brief File I/O utilities
 * @date 2024-02-24
 * @copyright Copyright (c) 2024
 */
#pragma once

#include "defines.h"
#include "str.h"

typedef struct str8_opt {
  str8 contents;
  bool has_value;
} str8_opt;

const char* string_from_file(const char* path);

str8_opt str8_from_file(arena* a, str8 path);

typedef struct {
    char *data;
    size_t size;
} FileData;

FileData load_spv_file(const char *path);