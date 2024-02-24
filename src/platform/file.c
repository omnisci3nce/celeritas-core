#include "file.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

const char *string_from_file(const char *path) {
  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    ERROR("Error reading file: %s. errno: %d", path, errno);
    return NULL;
  }
  if (ferror(f)) {
    ERROR("Error reading file: %s. errno: %d", path, errno);
    return NULL;
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  rewind(f);

  char *string = malloc(fsize + 1);
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = '\0';

  return string;
}