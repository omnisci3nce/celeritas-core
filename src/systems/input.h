/**
 * @brief
 */
#pragma once

#include "defines.h"
#include "keys.h"

struct core;
struct GLFWWindow;

typedef struct mouse_state {
  i32 x;
  i32 y;
  i32 x_delta;
  i32 y_delta;
  bool prev_left_btn_pressed;
  bool left_btn_pressed;
} mouse_state;

typedef struct input_state {
  struct GLFWwindow *window;
  mouse_state mouse;
  bool depressed_keys[KEYCODE_MAX];
  bool just_pressed_keys[KEYCODE_MAX];
  bool just_released_keys[KEYCODE_MAX];
} input_state;

/** @brief `key` is currently being held down */
bool key_is_pressed(keycode key);

/** @brief `key` was just pressed */
bool key_just_pressed(keycode key);

/** @brief `key` was just released */
bool key_just_released(keycode key);

// --- Lifecycle
bool input_system_init(input_state *input, struct GLFWwindow *window);
void input_system_shutdown(input_state *input);
void input_update(input_state *state);