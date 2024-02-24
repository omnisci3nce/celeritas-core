/**
 * @file mem.h
 * @brief Allocators, memory tracking
 * @version 0.1
 * @date 2024-02-24
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "defines.h"

typedef void* (*alloc)(size_t amount);