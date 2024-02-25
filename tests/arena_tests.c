/*
size_t arena_size = 16;
  arena scratch = arena_create(malloc(arena_size), arena_size);

  i32* int_ptr = arena_alloc(&scratch, sizeof(i32));
  i32* b = arena_alloc(&scratch, sizeof(i32));
  i32* c = arena_alloc(&scratch, sizeof(i32));
  *int_ptr = 55;
  printf("Int pointer %d %p\n", (*int_ptr), int_ptr);

  // will abort on second arena alloc
*/