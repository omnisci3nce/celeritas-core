#include "path.h"

#include <libgen.h>
#include <string.h>
#include "str.h"

#if defined(CEL_PLATFORM_LINUX) || defined(CEL_PLATFORM_MAC)
path_opt path_parent(const char* path) {
  char* path_dirname = dirname(path);
  return (path_opt){ .path = str8_cstr_view(path_dirname), .has_value = true };
}
#endif
#ifdef CEL_PLATFORM_WINDOWS
// TODO: path_opt path_parent(const char* path)
#endif