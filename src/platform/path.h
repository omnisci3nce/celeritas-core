/**
 * @file path.h
 * @brief
 * @date 2024-03-11
 * @copyright Copyright (c) 2024
 */
#pragma once

#include "str.h"

typedef struct path_opt {
  str8 path;
  bool has_value;
} path_opt;

path_opt path_parent(const char* path);  // TODO: convert to using str8