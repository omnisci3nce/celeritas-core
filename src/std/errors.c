#include <stdlib.h>
#include "defines.h"
#include "stdio.h"

#define B_STACKTRACE_IMPL
#include <b_stacktrace.h>

core_noreturn void core_abort() {
  crash_handler();
}

core_noreturn void crash_handler() {
  char* stacktrace = b_stacktrace_get_string();

  printf("stacktrace: %s\n", stacktrace);

  app_exit(1);
}

core_noreturn void app_exit(int code) {
  exit(code);
}