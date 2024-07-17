#include "str.h"
#include <assert.h>
#include <string.h>
#include "mem.h"

Str8 Str8_create(u8* buf, size_t len) { return (Str8){ .buf = buf, .len = len }; }

Str8 Str8_cstr_view(char* string) { return Str8_create((u8*)string, strlen(string)); }

bool Str8_equals(Str8 a, Str8 b) {
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

char* Str8_to_cstr(arena* a, Str8 s) {
  bool is_null_terminated = s.buf[s.len - 1] == 0;
  size_t n_bytes = is_null_terminated ? s.len : s.len + 1;

  u8* dest = arena_alloc(a, n_bytes);

  memcpy(dest, s.buf, s.len);
  if (is_null_terminated) {
    dest[s.len] = '\0';
  }
  return (char*)dest;
}

Str8 Str8_concat(arena* a, Str8 left, Str8 right) {
  size_t n_bytes = left.len + right.len + 1;

  u8* dest = arena_alloc(a, n_bytes);
  memcpy(dest, left.buf, left.len);
  memcpy(dest + right.len, right.buf, right.len);

  dest[n_bytes - 1] = '\0';

  return Str8_create(dest, n_bytes);
}

Str8 Str8_substr(Str8 s, u64 min, u64 max) {
  assert(min >= 0);
  assert(min < s.len);
  assert(max >= 0);
  assert(max <= s.len);
  uint8_t* start = s.buf + (ptrdiff_t)min;
  size_t new_len = max - min;
  return (Str8){ .buf = start, .len = new_len };
}

Str8 Str8_take(Str8 s, u64 first_n) { return Str8_substr(s, 0, first_n); }

Str8 Str8_drop(Str8 s, u64 last_n) { return Str8_substr(s, s.len - last_n, s.len); }

Str8 Str8_skip(Str8 s, u64 n) { return Str8_substr(s, n, s.len); }

Str8 Str8_chop(Str8 s, u64 n) { return Str8_substr(s, 0, s.len - n); }
