#include <glfw3.h>

#include "core.h"
#include "file.h"
#include "render.h"
#include "str.h"

int main() {
  core* core = core_bringup();

  // Examples of how to work with arenas and strings
  size_t arena_size = 1024;
  arena scratch = arena_create(malloc(arena_size), arena_size);
  arena* a = &scratch;

  str8 hello = str8lit("Hello World");

  // this works but we should be careful because str8 is not *guaranteed* to point to
  // a null-terminated string
  printf("String before: '%s' (null-terminated: %s) \n ", hello.buf,
         str8_is_null_term(hello) ? "true" : "false");

  char* c = str8_to_cstr(&scratch, hello);

  printf("String after: %s\n", c);

  str8_opt test_file = str8_from_file(&scratch, str8lit("assets/shaders/ui_rect.vert"));
  if (test_file.has_value) {
    printf("Contents: %.*s \n", (int)test_file.contents.len, test_file.contents.buf);
    printf("Null-terminated: %s\n", str8_is_null_term(test_file.contents) ? "true" : "false");
  }

  // Main loop
  while (!glfwWindowShouldClose(core->renderer.window)) {
    input_update(&core->input);
    threadpool_process_results(&core->threadpool, 1);

    render_frame_begin(&core->renderer);

    // insert work here

    render_frame_end(&core->renderer);
  }

  return 0;
}
