#include "file.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "mem.h"
#include "str.h"

const char *string_from_file(const char *path) {
  FILE *f = fopen(path, "rb");
  printf("Hello\n");
  if (f == NULL) {
    ERROR("Error reading file: %s. errno: %d", path, errno);
    return NULL;
  }
  if (ferror(f)) {
    ERROR("Error reading file: %s. errno: %d", path, errno);
    return NULL;
  }
  printf("Hello2\n");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  rewind(f);

  char *string = malloc(fsize + 1);
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = '\0';
  printf("Hello3\n");
  printf("Hello %s\n", string);

  return string;
}

str8_opt str8_from_file(arena *a, str8 path) {
  char *p = cstr(a, path);
  str8_opt result = { .has_value = false };

  FILE *f = fopen(p, "rb");
  if (f == NULL) {
    ERROR("Error reading file: %s. errno: %d", path, errno);
    return result;
  }
  if (ferror(f)) {
    ERROR("Error reading file: %s. errno: %d", path, errno);
    return result;
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  rewind(f);

  u8 *raw = arena_alloc(a, fsize + 1);
  str8 contents = str8_create(raw, fsize);
  contents.buf[contents.len] = '\0';

  fread(raw, fsize, 1, f);
  fclose(f);
  result.contents = contents;
  result.has_value = true;

  return result;
}
