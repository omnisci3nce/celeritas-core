/**
 * @brief Drawing shapes for UI or other reasons in screenspace
 */
#pragma once

#include "colours.h"
#include "darray.h"
#include "defines.h"
#include "render_types.h"

/** A draw_cmd packet for rendering a rectangle */
struct draw_rect {
  i32 x, y;  // signed ints so we can draw things offscreen (e.g. a window half inside the viewport)
  u32 width, height;
  rgba colour;
  // TODO: border colour, gradients
};

/** A draw_cmd packet for rendering a circle */
struct draw_circle {
  i32 x, y;
  f32 radius;
  rgba colour;
};

/** @brief Tagged union that represents a UI shape to be drawn. */
typedef struct draw_cmd {
  enum { RECT, CIRCLE } draw_cmd_type;
  union {
    struct draw_rect rect;
    struct draw_circle circle;
  };
} draw_cmd;

KITC_DECL_TYPED_ARRAY(draw_cmd)

typedef struct screenspace_state {
  u32 rect_vbo;
  u32 rect_vao;
  // shader rect_shader;
  draw_cmd_darray* draw_cmd_buf;
} screenspace_state;

// --- Lifecycle
bool screenspace_2d_init(screenspace_state* state);
void screenspace_2d_shutdown(screenspace_state* state);
/** Drains the draw_cmd buffer and emits draw calls to render each one */
void screenspace_2d_render(screenspace_state* state);

struct core;

/** @brief Draw a rectangle to the screen. (0,0) is the bottom-left */
void draw_rectangle(struct core* core, rgba colour, i32 x, i32 y, u32 width, u32 height);