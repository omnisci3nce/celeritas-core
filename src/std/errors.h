#pragma once

#include "defines.h"
#include "stdio.h"

#define CORE_ABORT(...) core_abort(__FILE__, __LINE__, __VA_ARGS__)

#define ERROR_EXIT(...)           \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
    exit(1);                      \
  }

core_noreturn void app_exit(int code);
core_noreturn void crash_handler();