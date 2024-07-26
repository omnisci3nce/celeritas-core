#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define ERROR_EXIT(...)           \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
    exit(1);                      \
  }

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1

#ifdef CRELEASE
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#else
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1
#endif

typedef enum log_level {
  LOG_LEVEL_FATAL = 0,
  LOG_LEVEL_ERROR = 1,
  LOG_LEVEL_WARN = 2,
  LOG_LEVEL_INFO = 3,
  LOG_LEVEL_DEBUG = 4,
  LOG_LEVEL_TRACE = 5,
} log_level;

bool logger_init();
void logger_shutdown();

// TODO: macro that outputs logger macros for a specific subsystem or string prefix e.g. "MEMORY" ->
// logs now have more context potentially have line numbers too?

void log_output(log_level level, const char *message, ...);

#define FATAL(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)
#define ERROR(message, ...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)
#define WARN(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#define INFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__)

#if LOG_DEBUG_ENABLED == 1
#define DEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#else
#define DEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define TRACE(message, ...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#else
#define TRACE(message, ...)
#endif

// TODO: Move this to an asserts file

void report_assertion_failure(const char *expression, const char *message, const char *file,
                              int line);

#define CASSERT(expr)                                          \
  {                                                            \
    if (expr) {                                                \
    } else {                                                   \
      report_assertion_failure(#expr, "", __FILE__, __LINE__); \
      __builtin_trap();                                        \
    }                                                          \
  }

#define CASSERT_MSG(expr, msg)                                  \
  {                                                             \
    if (expr) {                                                 \
    } else {                                                    \
      report_assertion_failure(#expr, msg, __FILE__, __LINE__); \
      __builtin_trap();                                         \
    }                                                           \
  }