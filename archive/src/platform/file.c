#include "file.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "mem.h"
#include "str.h"

const char* string_from_file(const char* path) {
  FILE* f = fopen(path, "rb");
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

  char* string = malloc(fsize + 1);
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = '\0';

  return string;
}

str8_opt str8_from_file(arena* a, Str8 path) {
  char* p = cstr(a, path);
  str8_opt result = { .has_value = false };

  FILE* f = fopen(p, "rb");
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

  u8* raw = arena_alloc(a, fsize + 1);
  Str8 contents = Str8_create(raw, fsize);
  contents.buf[contents.len] = '\0';

  fread(raw, fsize, 1, f);
  fclose(f);
  result.contents = contents;
  result.has_value = true;

  return result;
}

FileData load_spv_file(const char* path) {
  FILE* f = fopen(path, "rb");
  if (f == NULL) {
    perror("Error opening file");
    return (FileData){ NULL, 0 };
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  rewind(f);

  char* data = (char*)malloc(fsize);
  if (data == NULL) {
    perror("Memory allocation failed");
    fclose(f);
    return (FileData){ NULL, 0 };
  }

  size_t bytesRead = fread(data, 1, fsize, f);
  if (bytesRead < fsize) {
    perror("Failed to read the entire file");
    free(data);
    fclose(f);
    return (FileData){ NULL, 0 };
  }

  fclose(f);
  return (FileData){ data, bytesRead };
}
