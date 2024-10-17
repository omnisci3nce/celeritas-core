#include <celeritas.h>

static const char* log_level_strings[] = {
  "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"
};

void log_output(char* module, loglevel level, const char* message, ...) {
  char out_msg[4096];

  printf("[%s] %s Msg: %s\n", module, log_level_strings[level], message);
}
