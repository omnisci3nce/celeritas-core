#include <celeritas.h>

void log_output(char* module, LogLevel level, const char* message, ...) {
  char out_msg[4096];

  printf("Msg: %s\n", message);
}
