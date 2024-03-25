#pragma once

#include "defines.h"

typedef struct rgba {
  f32 r, g, b, a;
} rgba;

#define COLOUR_BLACK ((rgba){ 0.02, 0.02, 0.0, 1.0 })
#define COLOUR_IMPERIAL_RED ((rgba){ 0.97, 0.09, 0.21, 1.0 })
#define COLOUR_TRUE_BLUE ((rgba){ 0.11, 0.41, 0.77, 1.0 })
#define COLOUR_SEA_GREEN ((rgba){ 0.18, 0.77, 0.71, 1.0 })
#define COLOUR_WHITE ((rgba){ 1.0, 1.0, 1.0, 1.0 })

#define rgba_to_vec4(color) (vec4(color.r, color.g, color.b, color.a))

// Thanks ChatGPT
#define STONE_50 ((rgba){ 0.980, 0.980, 0.976, 1.0 })
#define STONE_100 ((rgba){ 0.961, 0.961, 0.957, 1.0 })
#define STONE_200 ((rgba){ 0.906, 0.898, 0.894, 1.0 })
#define STONE_300 ((rgba){ 0.839, 0.827, 0.819, 1.0 })
#define STONE_400 ((rgba){ 0.659, 0.635, 0.620, 1.0 })
#define STONE_500 ((rgba){ 0.471, 0.443, 0.424, 1.0 })
#define STONE_600 ((rgba){ 0.341, 0.325, 0.306, 1.0 })
#define STONE_700 ((rgba){ 0.267, 0.251, 0.235, 1.0 })
#define STONE_800 ((rgba){ 0.161, 0.145, 0.141, 1.0 })
#define STONE_900 ((rgba){ 0.110, 0.098, 0.090, 1.0 })
#define STONE_950 ((rgba){ 0.047, 0.043, 0.035, 1.0 })

#define CYAN_50 ((rgba){ 0.930, 1.000, 1.000, 1.0 })
#define CYAN_100 ((rgba){ 0.810, 0.980, 1.000, 1.0 })
#define CYAN_200 ((rgba){ 0.650, 0.953, 0.988, 1.0 })
#define CYAN_300 ((rgba){ 0.404, 0.910, 0.976, 1.0 })
#define CYAN_400 ((rgba){ 0.133, 0.827, 0.933, 1.0 })
#define CYAN_500 ((rgba){ 0.023, 0.714, 0.831, 1.0 })
#define CYAN_600 ((rgba){ 0.031, 0.569, 0.698, 1.0 })
#define CYAN_700 ((rgba){ 0.055, 0.455, 0.565, 1.0 })
#define CYAN_800 ((rgba){ 0.082, 0.369, 0.459, 1.0 })
#define CYAN_900 ((rgba){ 0.086, 0.306, 0.388, 1.0 })
#define CYAN_950 ((rgba){ 0.033, 0.200, 0.263, 1.0 })
