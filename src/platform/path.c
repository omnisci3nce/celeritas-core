#include "path.h"

#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "str.h"

#if defined(CEL_PLATFORM_LINUX) || defined(CEL_PLATFORM_MAC)
#include <libgen.h>
path_opt path_parent(arena* a, const char* path) {
  // Duplicate the string because dirname doesnt like const literals
  char* path_copy = arena_alloc(a, strlen(path) + 1);
  strcpy(path_copy, path);
  char* path_dirname = dirname(path_copy);
  return (path_opt){ .path = Str8_cstr_view(path_dirname), .has_value = true };
}
#endif
#ifdef CEL_PLATFORM_WINDOWS
#include <shlwapi.h>
#include <windows.h>
#pragma comment(lib, "Shlwapi.lib")

path_opt path_parent(arena* a, const char* path) {
  // Duplicate the string because PathRemoveFileSpec mutates in-place
  size_t len = strlen(path) + 1;
  char* path_copy = arena_alloc(a, len);
  strcpy_s(path_copy, len, path);

  if (PathRemoveFileSpecA(path_copy)) {
    return (path_opt){ .path = Str8_cstr_view(path_copy), .has_value = true };
  } else {
    return (path_opt){ .has_value = false };
  }
}
#endif
