#include "str.h"
#include <assert.h>
#include <string.h>
#include "mem.h"

str8 str8_create(u8* buf, size_t len) { return (str8){ .buf = buf, .len = len }; }

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

char* str8_to_cstr(arena* a, str8 s) {
  bool is_null_terminated = s.buf[s.len - 1] == 0;
  size_t n_bytes = is_null_terminated ? s.len : s.len + 1;

  u8* dest = arena_alloc(a, n_bytes);

  memcpy(dest, s.buf, s.len);
  if (is_null_terminated) {
    dest[s.len] = '\0';
  }
  return (char*)dest;
}

str8 str8_concat(arena* a, str8 left, str8 right) {
  size_t n_bytes = left.len + right.len + 1;

  u8* dest = arena_alloc(a, n_bytes);
  memcpy(dest, left.buf, left.len);
  memcpy(dest + right.len, right.buf, right.len);

  dest[n_bytes - 1] = '\0';

  return str8_create(dest, n_bytes);
}

str8 str8_substr(str8 s, u64 min, u64 max) {
  assert(min >= 0);
  assert(min < s.len);
  assert(max >= 0);
  assert(max <= s.len);
  uint8_t* start = s.buf + (ptrdiff_t)min;
  size_t new_len = max - min;
  return (str8){ .buf = start, .len = new_len };
}

str8 str8_take(str8 s, u64 first_n) { return str8_substr(s, 0, first_n); }

str8 str8_drop(str8 s, u64 last_n) { return str8_substr(s, s.len - last_n, s.len); }

str8 str8_skip(str8 s, u64 n) { return str8_substr(s, n, s.len); }

str8 str8_chop(str8 s, u64 n) { return str8_substr(s, 0, s.len - n); }
