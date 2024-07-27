/* The Goal of this file is to test ocaml-bindgen on it to start moving development over into OCaml */

// #include <stdbool.h>
// #include <stdint.h>

typedef struct Core Core;
typedef struct GLFWwindow GLFWwindow;

Core* get_global_core();
void core_Bringup(void* optional_window);

void frame_Begin();
void frame_Draw();
void frame_End();


struct Vec2 { float x; float y; };
typedef struct Vec3 { float x; float y; float z; } Vec3;
struct Vec4 { float x; float y; float z; float w; };

Vec3 vec3_add(Vec3 a, Vec3 b);