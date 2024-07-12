#include "input.h"

#include <assert.h>
#include <glfw3.h>
#include <string.h>

#include "log.h"

static Input_State *g_input;  // Use a global to simplify caller code

bool Input_Init(Input_State *input, GLFWwindow *window) {
  INFO("Input init");
  memset(input, 0, sizeof(Input_State));

  input->window = window;
  // Set everything to false. Could just set memory to zero but where's the fun in that
  for (int i = 0; i < KEYCODE_MAX; i++) {
    input->depressed_keys[i] = false;
    input->just_pressed_keys[i] = false;
    input->just_released_keys[i] = false;
  }

  g_input = input;

  assert(input->mouse.x_delta == 0);
  assert(input->mouse.y_delta == 0);

  INFO("Finish input init");
  return true;
}

void Input_Shutdown(Input_State *input) {}

void Input_Update(Input_State *input) {
  glfwPollEvents();
  // --- update keyboard input

  // if we go from un-pressed -> pressed, set as "just pressed"
  // if we go from pressed -> un-pressed, set as "just released"
  for (int i = 0; i < KEYCODE_MAX; i++) {
    bool new_state = false;
    if (glfwGetKey(input->window, i) == GLFW_PRESS) {
      new_state = true;
    } else {
      new_state = false;
    }
    if (!input->depressed_keys[i] == false && new_state) {
      input->just_pressed_keys[i] = true;
    } else {
      input->just_pressed_keys[i] = false;
    }

    if (input->depressed_keys[i] && !new_state) {
      input->just_released_keys[i] = true;
    } else {
      input->just_released_keys[i] = false;
    }

    input->depressed_keys[i] = new_state;
  }

  // --- update mouse input

  // cursor position
  f64 current_x, current_y;
  glfwGetCursorPos(input->window, &current_x, &current_y);
  i32 quantised_cur_x = (i32)current_x;
  i32 quantised_cur_y = (i32)current_y;

  mouse_state new_mouse_state = { 0 };
  new_mouse_state.x = quantised_cur_x;
  new_mouse_state.y = quantised_cur_y;
  new_mouse_state.x_delta = quantised_cur_x - input->mouse.x;
  new_mouse_state.y_delta = quantised_cur_y - input->mouse.y;

  // buttons
  int left_state = glfwGetMouseButton(input->window, GLFW_MOUSE_BUTTON_LEFT);
  // int right_state = glfwGetMouseButton(input->window, GLFW_MOUSE_BUTTON_RIGHT);

  new_mouse_state.prev_left_btn_pressed = input->mouse.left_btn_pressed;
  if (left_state == GLFW_PRESS) {
    new_mouse_state.left_btn_pressed = true;
  } else {
    new_mouse_state.left_btn_pressed = false;
  }

  // this was dumb! need to also check button state changes lol
  // if (new_mouse_state.x != input->mouse.x || new_mouse_state.y != input->mouse.y)
  // TRACE("Mouse (x,y) = (%d,%d)", input->mouse.x, input->mouse.y);

  input->mouse = new_mouse_state;
}

bool key_is_pressed(keycode key) { return g_input->depressed_keys[key]; }

bool key_just_pressed(keycode key) { return g_input->just_pressed_keys[key]; }

bool key_just_released(keycode key) { return g_input->just_released_keys[key]; }
