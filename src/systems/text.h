/**
 * @brief
 */
#pragma once

#include <stb_truetype.h>

#include "darray.h"
#include "defines.h"
#include "render_types.h"

struct core;
typedef struct texture_handle {
  u32 raw
} texture_handle;
typedef struct shader {
  u32 raw
} shader;

/** @brief internal font struct */
typedef struct font {
  const char *name;
  stbtt_fontinfo stbtt_font;
  stbtt_bakedchar c_data[96];
  texture_handle bitmap_tex;
} font;

typedef struct draw_text_packet {
  char *contents;
  f32 x;
  f32 y;
} draw_text_packet;

KITC_DECL_TYPED_ARRAY(draw_text_packet)

typedef struct text_system_state {
  font default_font;
  shader glyph_shader;
  u32 glyph_vbo;
  u32 glyph_vao;
  draw_text_packet_darray *draw_cmd_buf;
  // TODO: fonts array or hashtable
} text_system_state;

void text_system_render(text_system_state *text);

// --- Lifecycle functions
bool text_system_init(text_system_state *text);
void text_system_shutdown(text_system_state *text);

// --- Drawing

/**
 * @brief immediate mode draw text.
 * @note immediately emits draw calls causing a shader program switch if you weren't previously
         drawing text in the current frame.
*/
void draw_text(struct core *core, f32 x, f32 y, char *contents);