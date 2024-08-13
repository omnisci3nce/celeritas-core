#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "defines.h"

// Regular text
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"
#define CRESET "\e[0m"

static const char* level_strings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]: ",
                                        "[INFO]: ",  "[DEBUG]: ", "[TRACE]: " };
static const char* level_colours[6] = { RED, RED, YEL, BLU, CYN, MAG };

bool logger_init() {
  // TODO: create log file
  return true;
}

void logger_shutdown() {
  // does nothing right now
}

void log_output(log_level level, const char* message, ...) {
  char out_message[32000];
  memset(out_message, 0, sizeof(out_message));

  // format original message
  __builtin_va_list arg_ptr;
  va_start(arg_ptr, message);
  vsnprintf(out_message, 32000, message, arg_ptr);
  va_end(arg_ptr);

  char out_message2[32006];
  // prepend log level string
  sprintf(out_message2, "%s%s%s%s\n", level_colours[level], level_strings[level], out_message,
          CRESET);

  // print message to console
  printf("%s", out_message2);
}

void report_assertion_failure(const char* expression, const char* message, const char* file,
                              int line) {
  log_output(LOG_LEVEL_FATAL, "Assertion failure: %s, message: '%s', in file: %s, on line %d\n",
             expression, message, file, line);
}
