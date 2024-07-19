/**
 * @brief
 */
#pragma once

#include "defines.h"
#include "keys.h"

struct GLFWWindow;

typedef enum MouseBtn {
  MOUSEBTN_LEFT = 0,
  MOUSEBTN_RIGHT = 1,
  MOUSEBTN_MIDDLE = 2,
} MouseBtn;

typedef struct mouse_state {
  i32 x;
  i32 y;
  i32 x_delta;
  i32 y_delta;
  bool prev_pressed_states[3];
  bool cur_pressed_states[3];
} mouse_state;

typedef struct Input_State {
  struct GLFWwindow *window;
  mouse_state mouse;
  bool depressed_keys[KEYCODE_MAX];
  bool just_pressed_keys[KEYCODE_MAX];
  bool just_released_keys[KEYCODE_MAX];
} Input_State;

/** @brief `key` is currently being held down */
bool key_is_pressed(keycode key);

/** @brief `key` was just pressed */
bool key_just_pressed(keycode key);

/** @brief `key` was just released */
bool key_just_released(keycode key);

// TODO: right btn as well
bool MouseBtn_Held(MouseBtn btn);

// --- Lifecycle

bool Input_Init(Input_State *input, struct GLFWwindow *window);
void Input_Shutdown(Input_State *input);

void Input_Update(Input_State *state);  // must be run once per main loop

PUB mouse_state Input_GetMouseState();