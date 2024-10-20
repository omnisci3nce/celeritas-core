#include "platform.h"

#if defined(CEL_PLATFORM_LINUX) || defined(CEL_PLATFORM_MAC)

#include <libgen.h>
#include <string.h>

path_opt path_parent(arena* a, const char* path) {
  // Duplicate the string because dirname doesnt like const literals
  char* path_copy = arena_alloc(a, strlen(path) + 1);
  strcpy(path_copy, path);
  char* path_dirname = dirname(path_copy);
  return (path_opt){ .path = Str8_cstr_view(path_dirname), .has_value = true };
}

#endif
