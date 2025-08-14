/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --no-compress --font MiSans-Normal.ttf --symbols 0,1,2,3,4,5,6,7,8,9,～,开,始,游,戏,再,来,一,局,w,h,a,c,k,m,o,l,e,s,c,o,r,e,t,i,m,e,:
 --range 32-127 --format lvgl -o MiSans-Normal.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef MISANS_NORMAL
#define MISANS_NORMAL 1
#endif

#if MISANS_NORMAL

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0x88,

    /* U+0022 "\"" */
    0xb6, 0xd0,

    /* U+0023 "#" */
    0x9, 0x2, 0x41, 0x90, 0x44, 0x7f, 0xc4, 0x81,
    0x20, 0x48, 0xff, 0x88, 0x82, 0x60, 0x90, 0x24,
    0x0,

    /* U+0024 "$" */
    0x8, 0x8, 0x7e, 0xcb, 0x88, 0x88, 0x68, 0x3c,
    0xe, 0x9, 0x9, 0x89, 0xea, 0x3e, 0x8, 0x8,

    /* U+0025 "%" */
    0x70, 0x24, 0xc3, 0x22, 0x31, 0x11, 0xd, 0x98,
    0x39, 0x80, 0x8, 0x0, 0x9e, 0xd, 0x98, 0x48,
    0x44, 0x42, 0x63, 0x32, 0xf, 0x0,

    /* U+0026 "&" */
    0x1c, 0xc, 0x82, 0x20, 0x88, 0x26, 0x7, 0x3,
    0x81, 0x22, 0x85, 0xa0, 0xc8, 0x31, 0x1e, 0x38,
    0x80,

    /* U+0027 "'" */
    0xf0,

    /* U+0028 "(" */
    0x69, 0x69, 0x24, 0x92, 0x64, 0x98,

    /* U+0029 ")" */
    0xc9, 0x32, 0x49, 0x24, 0xb4, 0xb0,

    /* U+002A "*" */
    0x27, 0xc9, 0xf2, 0x0,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x0,

    /* U+002C "," */
    0xe0,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x10, 0x84, 0x62, 0x11, 0x88, 0x42, 0x31, 0x8,
    0x0,

    /* U+0030 "0" */
    0x3c, 0x66, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x42, 0x66, 0x3c,

    /* U+0031 "1" */
    0x37, 0x91, 0x11, 0x11, 0x11, 0x11, 0x10,

    /* U+0032 "2" */
    0x39, 0x8a, 0x8, 0x10, 0x20, 0xc1, 0x4, 0x18,
    0x61, 0x86, 0xf, 0xe0,

    /* U+0033 "3" */
    0xfe, 0x8, 0x30, 0xc3, 0xf, 0x1, 0x1, 0x2,
    0x6, 0xe, 0x27, 0x80,

    /* U+0034 "4" */
    0x6, 0x6, 0xa, 0x1a, 0x12, 0x32, 0x22, 0x42,
    0xc2, 0xff, 0x2, 0x2, 0x2,

    /* U+0035 "5" */
    0x7e, 0x40, 0x40, 0x40, 0x40, 0x7c, 0x62, 0x1,
    0x1, 0x1, 0xc1, 0x62, 0x3c,

    /* U+0036 "6" */
    0x8, 0x18, 0x30, 0x20, 0x60, 0x7c, 0xc2, 0x81,
    0x81, 0x81, 0x81, 0x42, 0x3c,

    /* U+0037 "7" */
    0xfe, 0xc, 0x10, 0x20, 0xc1, 0x6, 0x8, 0x10,
    0x60, 0x81, 0x6, 0x0,

    /* U+0038 "8" */
    0x3c, 0x64, 0x42, 0x42, 0x42, 0x24, 0x18, 0x66,
    0x81, 0x81, 0x81, 0x42, 0x3c,

    /* U+0039 "9" */
    0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x43, 0x3e,
    0x6, 0x4, 0xc, 0x18, 0x10,

    /* U+003A ":" */
    0x80, 0x80,

    /* U+003B ";" */
    0x80, 0xe0,

    /* U+003C "<" */
    0x2, 0xc, 0x63, 0xc, 0xc, 0xe, 0x6, 0x2,

    /* U+003D "=" */
    0xfe, 0x0, 0x0, 0xf, 0xe0,

    /* U+003E ">" */
    0x81, 0x80, 0xc0, 0x60, 0x61, 0x8e, 0x30, 0x80,

    /* U+003F "?" */
    0x3c, 0x8e, 0x8, 0x10, 0x60, 0x82, 0x8, 0x10,
    0x20, 0x0, 0x1, 0x0,

    /* U+0040 "@" */
    0xf, 0x83, 0xc, 0x60, 0x24, 0x3, 0x8e, 0x99,
    0x99, 0x90, 0x99, 0x19, 0x8f, 0x64, 0x0, 0x40,
    0x3, 0x8, 0xf, 0x80,

    /* U+0041 "A" */
    0xc, 0x3, 0x0, 0xe0, 0x68, 0x1a, 0x4, 0xc3,
    0x10, 0xc4, 0x3f, 0x98, 0x24, 0x9, 0x3, 0xc0,
    0x40,

    /* U+0042 "B" */
    0xfc, 0x82, 0x81, 0x81, 0x81, 0x82, 0xfc, 0x82,
    0x81, 0x81, 0x81, 0x82, 0xfc,

    /* U+0043 "C" */
    0x1f, 0x10, 0xd0, 0x18, 0x8, 0x4, 0x2, 0x1,
    0x0, 0x80, 0x60, 0x10, 0x24, 0x31, 0xe0,

    /* U+0044 "D" */
    0xfe, 0x20, 0xc8, 0xa, 0x2, 0x80, 0x60, 0x18,
    0x6, 0x1, 0x80, 0x60, 0x28, 0xa, 0xc, 0xfc,
    0x0,

    /* U+0045 "E" */
    0xff, 0x2, 0x4, 0x8, 0x10, 0x3f, 0xc0, 0x81,
    0x2, 0x4, 0xf, 0xe0,

    /* U+0046 "F" */
    0xff, 0x2, 0x4, 0x8, 0x10, 0x3f, 0x40, 0x81,
    0x2, 0x4, 0x8, 0x0,

    /* U+0047 "G" */
    0x1f, 0xc, 0x24, 0x3, 0x0, 0x80, 0x20, 0x8,
    0x3e, 0x1, 0x80, 0x70, 0x14, 0x4, 0x86, 0x1f,
    0x0,

    /* U+0048 "H" */
    0x80, 0xc0, 0x60, 0x30, 0x18, 0xc, 0x7, 0xff,
    0x1, 0x80, 0xc0, 0x60, 0x30, 0x18, 0x8,

    /* U+0049 "I" */
    0xff, 0xf8,

    /* U+004A "J" */
    0x4, 0x10, 0x41, 0x4, 0x10, 0x41, 0x4, 0x14,
    0x53, 0x38,

    /* U+004B "K" */
    0x83, 0x43, 0x23, 0x13, 0xb, 0x7, 0x3, 0x81,
    0x60, 0x98, 0x46, 0x21, 0x10, 0x48, 0x30,

    /* U+004C "L" */
    0x81, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x81,
    0x2, 0x4, 0xf, 0xe0,

    /* U+004D "M" */
    0x80, 0x38, 0xf, 0x1, 0xf0, 0x5a, 0xb, 0x22,
    0x66, 0xcc, 0x51, 0x8e, 0x30, 0x86, 0x0, 0xc0,
    0x18, 0x2,

    /* U+004E "N" */
    0x80, 0xe0, 0x78, 0x34, 0x1b, 0xc, 0xc6, 0x23,
    0x19, 0x86, 0xc1, 0x60, 0xf0, 0x38, 0x8,

    /* U+004F "O" */
    0x1f, 0x4, 0x11, 0x1, 0x60, 0x28, 0x3, 0x0,
    0x60, 0xc, 0x1, 0x80, 0x38, 0x9, 0x1, 0x10,
    0x41, 0xf0,

    /* U+0050 "P" */
    0xfc, 0x82, 0x81, 0x81, 0x81, 0x81, 0x82, 0xfc,
    0x80, 0x80, 0x80, 0x80, 0x80,

    /* U+0051 "Q" */
    0x1f, 0x4, 0x11, 0x1, 0x60, 0x28, 0x3, 0x0,
    0x60, 0xc, 0x1, 0x80, 0x38, 0x2d, 0x7, 0x10,
    0x61, 0xf4, 0x0, 0x40,

    /* U+0052 "R" */
    0xfc, 0x82, 0x81, 0x81, 0x81, 0x82, 0xfc, 0x8c,
    0x84, 0x86, 0x82, 0x83, 0x81,

    /* U+0053 "S" */
    0x3c, 0x43, 0x80, 0x80, 0x80, 0x60, 0x3c, 0x6,
    0x1, 0x1, 0x81, 0xc2, 0x3c,

    /* U+0054 "T" */
    0xff, 0x84, 0x2, 0x1, 0x0, 0x80, 0x40, 0x20,
    0x10, 0x8, 0x4, 0x2, 0x1, 0x0, 0x80,

    /* U+0055 "U" */
    0x80, 0xc0, 0x60, 0x30, 0x18, 0xc, 0x6, 0x3,
    0x1, 0x80, 0xc0, 0x70, 0x6c, 0x63, 0xe0,

    /* U+0056 "V" */
    0xc0, 0x50, 0x34, 0x9, 0x82, 0x21, 0x88, 0x43,
    0x10, 0x4c, 0x12, 0x6, 0x80, 0xe0, 0x30, 0xc,
    0x0,

    /* U+0057 "W" */
    0xc1, 0x2, 0x83, 0xd, 0xe, 0x1a, 0x14, 0x26,
    0x28, 0x44, 0x49, 0x89, 0x92, 0x12, 0x24, 0x34,
    0x68, 0x28, 0x70, 0x60, 0xc0, 0xc1, 0x81, 0x83,
    0x0,

    /* U+0058 "X" */
    0x40, 0x98, 0x63, 0x10, 0x48, 0x1e, 0x3, 0x0,
    0xc0, 0x70, 0x12, 0xc, 0xc2, 0x11, 0x86, 0x40,
    0x80,

    /* U+0059 "Y" */
    0xc1, 0xa0, 0x98, 0xc4, 0x43, 0x60, 0xa0, 0x70,
    0x10, 0x8, 0x4, 0x2, 0x1, 0x0, 0x80,

    /* U+005A "Z" */
    0xff, 0x3, 0x2, 0x4, 0xc, 0x8, 0x18, 0x10,
    0x30, 0x20, 0x40, 0xc0, 0xff,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x49, 0x38,

    /* U+005C "\\" */
    0xc2, 0x10, 0x82, 0x10, 0x86, 0x10, 0x86, 0x10,
    0x80,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x92, 0x78,

    /* U+005E "^" */
    0x10, 0x70, 0xa2, 0x24, 0x40,

    /* U+005F "_" */
    0xfe,

    /* U+0060 "`" */
    0xb4,

    /* U+0061 "a" */
    0x7c, 0x8c, 0x8, 0x13, 0xf8, 0x60, 0xc3, 0x7a,

    /* U+0062 "b" */
    0x80, 0x80, 0x80, 0x80, 0xbc, 0xc2, 0x83, 0x81,
    0x81, 0x81, 0x83, 0xc2, 0xbc,

    /* U+0063 "c" */
    0x3c, 0x8e, 0x4, 0x8, 0x10, 0x20, 0x23, 0x3c,

    /* U+0064 "d" */
    0x1, 0x1, 0x1, 0x1, 0x3d, 0x43, 0xc3, 0x81,
    0x81, 0x81, 0x81, 0x43, 0x3d,

    /* U+0065 "e" */
    0x38, 0x8a, 0xc, 0x1f, 0xf0, 0x20, 0x23, 0x3c,

    /* U+0066 "f" */
    0x3a, 0x10, 0x8f, 0xa1, 0x8, 0x42, 0x10, 0x84,
    0x0,

    /* U+0067 "g" */
    0x3d, 0x43, 0xc1, 0x81, 0x81, 0x81, 0x81, 0x43,
    0x3d, 0x1, 0x1, 0xc2, 0x3c,

    /* U+0068 "h" */
    0x81, 0x2, 0x4, 0xb, 0xd8, 0xe0, 0xc1, 0x83,
    0x6, 0xc, 0x18, 0x20,

    /* U+0069 "i" */
    0x8f, 0xf8,

    /* U+006A "j" */
    0x20, 0x2, 0x49, 0x24, 0x92, 0x49, 0xc0,

    /* U+006B "k" */
    0x81, 0x2, 0x4, 0x8, 0xd3, 0x2c, 0x70, 0xe1,
    0x62, 0x64, 0x48, 0x40,

    /* U+006C "l" */
    0xaa, 0xaa, 0xaa, 0xc0,

    /* U+006D "m" */
    0xb9, 0xd9, 0xce, 0x10, 0xc2, 0x18, 0x43, 0x8,
    0x61, 0xc, 0x21, 0x84, 0x20,

    /* U+006E "n" */
    0xbd, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x82,

    /* U+006F "o" */
    0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81, 0x42,
    0x3c,

    /* U+0070 "p" */
    0xbc, 0xc2, 0xc3, 0x81, 0x81, 0x81, 0x83, 0xc2,
    0xbc, 0x80, 0x80, 0x80, 0x80,

    /* U+0071 "q" */
    0x3d, 0x43, 0xc3, 0x81, 0x81, 0x81, 0xc1, 0x43,
    0x3d, 0x1, 0x1, 0x1, 0x1,

    /* U+0072 "r" */
    0xbc, 0x88, 0x88, 0x88, 0x80,

    /* U+0073 "s" */
    0x7a, 0x38, 0x30, 0x78, 0x30, 0x61, 0x78,

    /* U+0074 "t" */
    0x42, 0x11, 0xf4, 0x21, 0x8, 0x42, 0x10, 0x70,

    /* U+0075 "u" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xe3, 0x7a,

    /* U+0076 "v" */
    0xc3, 0x42, 0x42, 0x66, 0x24, 0x24, 0x3c, 0x18,
    0x18,

    /* U+0077 "w" */
    0xc6, 0x14, 0x61, 0x45, 0x34, 0x52, 0x69, 0x22,
    0x9a, 0x28, 0xc3, 0x8c, 0x10, 0xc0,

    /* U+0078 "x" */
    0x42, 0x64, 0x24, 0x18, 0x18, 0x38, 0x24, 0x66,
    0xc2,

    /* U+0079 "y" */
    0xc1, 0x42, 0x42, 0x66, 0x24, 0x24, 0x3c, 0x18,
    0x18, 0x18, 0x10, 0x30, 0x20,

    /* U+007A "z" */
    0xfe, 0xc, 0x30, 0xc1, 0x6, 0x18, 0x60, 0xfe,

    /* U+007B "{" */
    0x19, 0x8, 0x42, 0x11, 0x98, 0x61, 0x8, 0x42,
    0x10, 0x60,

    /* U+007C "|" */
    0xff, 0xfe,

    /* U+007D "}" */
    0xc1, 0x8, 0x42, 0x10, 0xc3, 0x31, 0x8, 0x42,
    0x13, 0x0,

    /* U+007E "~" */
    0x67, 0x38,

    /* U+007F "" */
    0x0,

    /* U+4E00 "一" */
    0xff, 0xfc,

    /* U+518D "再" */
    0xff, 0xfc, 0xc, 0x3, 0xff, 0x8, 0xc4, 0x23,
    0x10, 0xff, 0xc2, 0x31, 0x8, 0xc4, 0xff, 0xfc,
    0x80, 0x42, 0x1, 0x8, 0x4, 0x20, 0xf0,

    /* U+59CB "始" */
    0x0, 0x0, 0x82, 0x4, 0x1b, 0x3e, 0x44, 0x4a,
    0x9, 0x2f, 0xe4, 0x80, 0x52, 0x0, 0x50, 0x3,
    0x4f, 0xe7, 0x20, 0x8c, 0x82, 0x3a, 0x9, 0xf,
    0xe8, 0x20, 0x80,

    /* U+5C40 "局" */
    0x3f, 0xf8, 0x80, 0x22, 0x0, 0x8f, 0xfe, 0x20,
    0x0, 0x80, 0x3, 0xff, 0xc8, 0x1, 0x2f, 0xc4,
    0xa1, 0x12, 0x84, 0x53, 0xf1, 0x40, 0xf, 0x3,
    0xe0,

    /* U+5F00 "开" */
    0x7f, 0xf8, 0x21, 0x80, 0x86, 0x2, 0x18, 0x8,
    0x63, 0xff, 0xf0, 0x86, 0x2, 0x18, 0x18, 0x60,
    0x41, 0x83, 0x6, 0x18, 0x18, 0x40, 0x60,

    /* U+620F "戏" */
    0x1, 0x0, 0xb, 0x3e, 0x4c, 0x12, 0x0, 0x9f,
    0xe7, 0xc1, 0x44, 0x46, 0x22, 0x30, 0xa1, 0x87,
    0xc, 0x30, 0x93, 0x14, 0xb4, 0xc3, 0x26, 0x0,
    0xc0,

    /* U+6765 "来" */
    0x3, 0x1, 0xff, 0xe0, 0x30, 0x8, 0xc4, 0x13,
    0x20, 0x6d, 0x80, 0x30, 0x3f, 0xff, 0x3, 0x0,
    0x3f, 0x1, 0xb6, 0x18, 0xc6, 0xc3, 0xc, 0xc,
    0x0,

    /* U+6E38 "游" */
    0x4, 0x44, 0x22, 0x19, 0x1e, 0x3f, 0xf0, 0x88,
    0x64, 0x3d, 0x38, 0x41, 0x44, 0xa, 0x22, 0x57,
    0xd2, 0x88, 0xa4, 0x4d, 0x22, 0x49, 0x12, 0xbb,
    0x80,

    /* U+FF5E "～" */
    0xe6, 0x70
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 91, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 67, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 85, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 5, .adv_w = 181, .box_w = 10, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 22, .adv_w = 147, .box_w = 8, .box_h = 16, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 38, .adv_w = 210, .box_w = 13, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 179, .box_w = 10, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 49, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 78, .adv_w = 77, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 84, .adv_w = 77, .box_w = 3, .box_h = 15, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 90, .adv_w = 113, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 94, .adv_w = 154, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 101, .adv_w = 65, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 102, .adv_w = 100, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 103, .adv_w = 62, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 100, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 161, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 101, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 144, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 151, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 150, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 153, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 154, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 133, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 159, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 152, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 62, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 236, .adv_w = 65, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 238, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 246, .adv_w = 154, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 251, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 259, .adv_w = 125, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 211, .box_w = 12, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 168, .box_w = 10, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 169, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 178, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 190, .box_w = 10, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 152, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 146, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 188, .box_w = 10, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 183, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 61, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 120, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 421, .adv_w = 164, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 144, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 448, .adv_w = 223, .box_w = 11, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 466, .adv_w = 187, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 481, .adv_w = 200, .box_w = 11, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 499, .adv_w = 158, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 512, .adv_w = 200, .box_w = 11, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 532, .adv_w = 161, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 545, .adv_w = 154, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 153, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 573, .adv_w = 179, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 588, .adv_w = 168, .box_w = 10, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 605, .adv_w = 245, .box_w = 15, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 630, .adv_w = 159, .box_w = 10, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 647, .adv_w = 158, .box_w = 9, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 662, .adv_w = 151, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 675, .adv_w = 89, .box_w = 3, .box_h = 15, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 681, .adv_w = 87, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 690, .adv_w = 89, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 696, .adv_w = 112, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 9},
    {.bitmap_index = 701, .adv_w = 116, .box_w = 7, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 702, .adv_w = 66, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 703, .adv_w = 136, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 711, .adv_w = 153, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 724, .adv_w = 136, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 732, .adv_w = 153, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 745, .adv_w = 143, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 753, .adv_w = 88, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 762, .adv_w = 153, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 775, .adv_w = 148, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 787, .adv_w = 62, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 789, .adv_w = 62, .box_w = 3, .box_h = 17, .ofs_x = -1, .ofs_y = -4},
    {.bitmap_index = 796, .adv_w = 129, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 808, .adv_w = 62, .box_w = 2, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 812, .adv_w = 226, .box_w = 11, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 825, .adv_w = 148, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 833, .adv_w = 147, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 842, .adv_w = 153, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 855, .adv_w = 153, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 868, .adv_w = 95, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 873, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 880, .adv_w = 89, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 888, .adv_w = 148, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 896, .adv_w = 129, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 201, .box_w = 12, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 919, .adv_w = 123, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 928, .adv_w = 130, .box_w = 8, .box_h = 13, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 941, .adv_w = 125, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 949, .adv_w = 89, .box_w = 5, .box_h = 15, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 959, .adv_w = 56, .box_w = 1, .box_h = 15, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 961, .adv_w = 89, .box_w = 5, .box_h = 15, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 971, .adv_w = 133, .box_w = 7, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 973, .adv_w = 0, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 974, .adv_w = 256, .box_w = 14, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 976, .adv_w = 256, .box_w = 14, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 999, .adv_w = 256, .box_w = 14, .box_h = 15, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1026, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1051, .adv_w = 256, .box_w = 14, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1074, .adv_w = 256, .box_w = 13, .box_h = 15, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1099, .adv_w = 256, .box_w = 14, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1124, .adv_w = 256, .box_w = 13, .box_h = 15, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1149, .adv_w = 256, .box_w = 6, .box_h = 2, .ofs_x = 5, .ofs_y = 5}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x38d, 0xbcb, 0xe40, 0x1100, 0x140f, 0x1965, 0x2038,
    0xb15e
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 96, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 19968, .range_length = 45407, .glyph_id_start = 97,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 9, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 2, 0, 2,
    3, 4, 0, 5, 6, 7, 0, 8,
    9, 10, 11, 0, 0, 0, 0, 0,
    0, 0, 12, 13, 14, 15, 16, 17,
    18, 0, 0, 19, 20, 21, 0, 0,
    15, 22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 0, 0, 0, 0,
    33, 0, 34, 35, 36, 0, 37, 38,
    39, 34, 0, 0, 40, 41, 34, 34,
    35, 35, 39, 42, 43, 44, 39, 45,
    46, 47, 45, 48, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 2, 0, 2,
    3, 4, 5, 6, 7, 8, 0, 9,
    10, 11, 12, 0, 0, 0, 0, 0,
    0, 0, 13, 0, 14, 0, 0, 0,
    14, 0, 0, 15, 0, 0, 0, 0,
    14, 0, 14, 0, 16, 17, 18, 19,
    20, 21, 22, 23, 0, 0, 0, 0,
    0, 0, 24, 0, 25, 25, 25, 26,
    25, 0, 0, 27, 0, 28, 29, 29,
    25, 29, 25, 29, 30, 31, 32, 33,
    34, 35, 33, 36, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, -7, 0,
    -20, -5, -18, -10, 0, -26, 0, -4,
    -2, 1, 0, -5, 0, 0, -2, 0,
    -9, -7, 0, 0, 0, 0, 0, 0,
    -15, 0, 0, -12, 0, 0, 0, -15,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -18,
    -13, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -8, 0, 0, 0, -5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -4, 0, 0, -7, -9,
    -5, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -5, 0, 0, 0, 0, 0, -3,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -5, 0,
    0, -7, 0, -5, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -8, 0, 0, 0, 0, -10, 0, -3,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -36, -24, -5, 0, 0, 0, -19,
    -17, 0, -5, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -5, 0, -4,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -28, -13, 0, 0, 0, 0, -6,
    -3, 0, -5, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -3, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -7, 0, 0, -34, -4, -27, -19,
    0, -30, 0, 0, -4, 0, 0, 0,
    0, 0, 0, 0, -12, -6, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -5, 0, 0, 0,
    -6, 0, -6, -5, -5, -9, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -3, 0, -1, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -5, 0, 0, -3, 0, 0, 0,
    -5, -3, -3, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -5, -5, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -8, 0, -15, 0,
    -8, 0, -9, -4, -13, -13, -9, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -3, -6, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -4, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -3, -3, 0, 0,
    -7, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -16, 0, -27, -3,
    0, 0, 0, 0, -5, 0, 0, -16,
    -7, 0, 0, 0, 0, 0, 0, -5,
    -3, -3, -10, -3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -6, 0, 0, 0, -8, 0, -10, -4,
    -7, -16, -3, 0, 0, 0, 0, 0,
    0, -2, 0, 0, -7, -8, -5, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -4, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -10, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -16, -11, -8, 0, -13, 0, 0,
    0, 0, 0, -8, -14, 0, 0, 0,
    0, -3, -13, -13, -20, -16, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -10, 0, 0,
    -35, -5, -31, -21, 0, -35, 0, 0,
    0, -6, 0, 0, 0, 0, -9, -6,
    -20, -13, 0, 0, -10, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -22, 0, -41, 0, -5, 0, -6, -5,
    -10, -8, -5, -14, -13, 0, 0, 0,
    0, -10, 0, -10, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -7, 0, 0, 0,
    -8, 0, -6, -6, -11, -13, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -4, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -2, -7, 0, -5, 0, -5, -3,
    0, -8, 0, -4, -5, 0, 0, 0,
    0, 0, 0, -5, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -6, 0, -3, 0, -7, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -5, -5, -3, -4, -15, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -34, -8, -40, -7, 0, 0, 0, 0,
    -7, 0, 0, -26, -26, -13, 0, 0,
    -23, -33, -5, -26, -28, -26, -28, -29,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -4, 0, -7, 0,
    0, 0, 0, 0, -4, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -18, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -27, -9, -40, -3, 0, 0, 0, 0,
    0, 0, 0, -22, -20, -9, 0, 0,
    -20, -19, -5, -19, -8, -8, -10, -11,
    -10, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -19, -4, -24, -2,
    0, 0, 0, 0, 0, 0, 0, -13,
    -11, -5, 0, 0, -12, -12, -3, -11,
    -8, -8, -10, -10, -5, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -13, 0, -3, -7, -4, 0, 0,
    0, 0, 0, -2, -10, -5, 0, -2,
    -4, -8, -13, -11, -21, -18, 0, 0,
    -20, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -30, -13, -44, -8,
    0, 0, 0, 0, 0, 0, 0, -39,
    -36, -13, 0, 0, -38, -33, -15, -36,
    -26, -25, -26, -28, -10, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -13, 0, 0, 0, -5, 0, 0,
    0, 0, 0, 0, -4, 0, 0, 0,
    0, 0, -7, -8, -11, -10, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 17, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -25, 0, -22, -13,
    -5, -40, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, -2, -2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -4, 0, 0, 0,
    -26, 0, -20, -11, -10, -36, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -3, -3, -8, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -26, 0, -12, -9,
    -5, -31, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, -5, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -26, 0, -16, -9, -8, -29, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, -1, -4, 0, -8, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -15, 0, -18, 0, 0, 0, 0, 0,
    0, 0, 0, -5, -4, 0, 0, 0,
    0, -4, 0, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -23, 0, -20, -14, 0, -36, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -4, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -5, 0, 0, -38, -3, -12, -8,
    0, -20, 0, -2, -4, 0, 0, -1,
    0, -3, -2, -4, 0, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -2,
    -2, -2, 0, 0, -13, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -23, 0, 0, 0,
    -20, -18, -13, -3, -3, 0, 0, 0,
    0, -2, 0, -4, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -32, 0, -13, -10, -8, -30, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -6, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -5, 0, -6, -3,
    0, -13, 0, 0, -2, 0, 0, 0,
    0, -2, 0, -1, 0, 0, 0, 0,
    -10, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -12, 0, -23, 0,
    -28, 0, -8, -8, -21, -26, -13, -2,
    -3, 0, 0, 0, 0, -2, 0, 0,
    0, 0, 0, 0, -6, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -6, 0, -19, 0, -26, 0, -8, -8,
    -18, -25, -10, -2, -3, 0, 0, 0,
    0, -2, 0, 0, 0, 0, 0, 0,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -3, 0, 0,
    -28, 0, -10, -10, 0, -26, 0, -4,
    -9, 0, 0, -3, 0, -6, 0, -3,
    0, 0, 0, 0, -9, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -8, 0, 0, -28, 0, -8, -12,
    0, -28, 0, -1, -4, 0, 0, 0,
    0, -2, 0, -2, 0, 0, 0, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 48,
    .right_class_cnt     = 36,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 1,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t MiSans_Normal = {
#else
lv_font_t MiSans_Normal = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 18,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if MISANS-NORMAL*/

