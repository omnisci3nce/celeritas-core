/**
 * @file buf.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-28
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include "defines.h"

typedef struct bytebuffer {
  u8* buf;
  size_t size;
} bytebuffer;
