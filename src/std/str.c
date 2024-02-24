#include "str.h"

bool str8_equals(str8 a, str8 b) {
  if (a.len != b.len) {
    return false;
  }

  for (size_t i = 0; i < a.len; i++) {
    if (a.buf[i] != b.buf[i]) {
      return false;
    }
  }
  return true;
}