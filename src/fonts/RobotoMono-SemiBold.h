/*
 * Roboto Mono SemiBold Font - Bitmap Data
 *
 * Original font: Roboto Mono SemiBold
 * Designer: Christian Robertson
 * Copyright: 2011 Google Inc.
 * License: Apache License 2.0
 *
 * Converted to bitmap format using LVGL font converter
 * https://lvgl.io/tools/fontconverter
 *
 * This bitmap data is derived from the original TTF font and is
 * distributed under the same Apache 2.0 license terms.
 */

#pragma once

#include "pico/stdlib.h"

struct font_dsc
{
    uint16_t bitmap_index;
    uint16_t adv_w;
    uint8_t box_w;
    uint8_t box_h;
    int8_t ofs_x;
    int8_t ofs_y;
};

typedef struct font_dsc font_dsc_t;

extern const uint8_t RobotoMono_SemiBold_glyph_bitmap[];

extern const font_dsc_t RobotoMono_SemiBold_glyph_dsc[];