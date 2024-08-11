#include <stdbool.h>
#include <stdlib.h>
#include "defines.h"
#include "unity.h"
#include "unity_fixture.h"

#include "mem.h"

#define ARENA_SIZE (1024 * 1024)

static arena a;

TEST_GROUP(Pool);

TEST_SETUP(Pool) { a = arena_create(malloc(ARENA_SIZE), ARENA_SIZE); }

TEST_TEAR_DOWN(Pool) { arena_free_all(&a); }

TEST(Pool, SanityCheckTest) { TEST_ASSERT_EQUAL(true, true); }

TEST(Pool, Initialisation) {
  // u32 pool
  void_pool pool = void_pool_create(&a, "Test pool", 256, sizeof(u32));
  u32 x_handle;
  u32* x_ptr = (u32*)void_pool_alloc(&pool, &x_handle);
  // store something in it
  *x_ptr = 1024;
  u32* x = void_pool_get(&pool, x_handle);

  TEST_ASSERT_EQUAL_UINT32(1024, *x);
  /* TEST_ASSERT_EQUAL_MEMORY(&expected, &actual, sizeof(vec3)); */
}

typedef struct foo {
  u32 a;
  f32 b;
  char c;
} foo;

CORE_DEFINE_HANDLE(bar);
typedef struct barHandle {
  u32 raw;
} barHandle;
TYPED_POOL(foo, bar);

TEST(Pool, TypedPool) {
  printf("Typed pool test\n");
  // create pool
  bar_pool pool = bar_pool_create(&a, 2, sizeof(foo));

  barHandle first_handle, second_handle, third_handle;
  foo* first = bar_pool_alloc(&pool, &first_handle);
  foo* second = bar_pool_alloc(&pool, &second_handle);
  // Third one shouldnt work
  foo* third = bar_pool_alloc(&pool, &third_handle);
  TEST_ASSERT_NULL(third);

  first->a = 32;
  first->b = 2.0;
  first->c = 'X';

  foo* my_foo = bar_pool_get(&pool, first_handle);
  TEST_ASSERT_EQUAL_UINT32(32, my_foo->a);
  TEST_ASSERT_EQUAL(2.0, my_foo->b);
  TEST_ASSERT_EQUAL_CHAR('X', my_foo->c);

  bar_pool_dealloc(&pool, first_handle);

  // next alloc should succeed
  third = bar_pool_alloc(&pool, &third_handle);
  TEST_ASSERT_NOT_NULL(third);
}
