/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: --bpp 1 --size 10 --no-compress --font SourceHanSansCN-Bold.ttf --symbols 父、亲、夫、丈、儿、子、哥、姐、母、妻、女、弟、妹、自、己、祖、伯、姑、堂、曾、外、舅、姨、侄、媳、婿、孙、姥、公、奶、爷、甥、嫡、庶、继、养、岳、妯、娌、玄、叔、清除、计算、<-、\n、空格、请、输、入、选、择、确、定、返、回、主、菜、单、计、算、结、果、关、系、帮、助、说、明、界、面、查、询、显、示、祖母、伯父、姑母、堂兄弟、堂姐妹、曾祖父、曾祖母、外祖父、外祖母、外曾祖父、外曾祖母、表侄、表侄女、再从兄弟、再从姐妹、叔祖父、姑祖母、舅父、姨母、侄子、侄女、儿媳、女婿、孙子、孙女、曾孙子、曾孙女、堂侄 / 表侄、堂侄女 / 表侄女、孙侄、孙侄女、未知、0、1、2、3、4、5、6、7、8、9、+、-、×、÷、=、？、！、，、。、：、；、《》、[]、{}、@、#、←、→、↑、↓、√、×、这、是、一、个、亲、属、关、系、计、算、器、。、未、支、持、关、系、：、“、未、知、”、。的 --range 32-127 --format lvgl -o lv_font_siyuan_10.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_SIYUAN_10
#define LV_FONT_SIYUAN_10 1
#endif

#if LV_FONT_SIYUAN_10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xaa, 0x8f,

    /* U+0022 "\"" */
    0xff, 0xf0,

    /* U+0023 "#" */
    0x52, 0xbe, 0xa5, 0x7e, 0x52,

    /* U+0024 "$" */
    0x23, 0xb1, 0x8f, 0x1c, 0x73, 0xf1, 0x0,

    /* U+0025 "%" */
    0x64, 0x4a, 0x25, 0x13, 0x66, 0xc8, 0xa4, 0x52,
    0x26,

    /* U+0026 "&" */
    0x38, 0x91, 0x61, 0x87, 0x3b, 0xf3, 0x3d,

    /* U+0027 "'" */
    0xfc,

    /* U+0028 "(" */
    0x5a, 0xaa, 0x94,

    /* U+0029 ")" */
    0xa5, 0x55, 0x68,

    /* U+002A "*" */
    0x23, 0x88, 0xa0,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+002C "," */
    0xf6,

    /* U+002D "-" */
    0xc0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x22, 0x22, 0x24, 0x44, 0x48, 0x80,

    /* U+0030 "0" */
    0x76, 0xf7, 0xbd, 0xef, 0x6e,

    /* U+0031 "1" */
    0x6e, 0x66, 0x66, 0x6f,

    /* U+0032 "2" */
    0x74, 0xc6, 0x33, 0x19, 0x9f,

    /* U+0033 "3" */
    0x72, 0x42, 0x60, 0x85, 0x2e,

    /* U+0034 "4" */
    0x39, 0xde, 0xbd, 0xfc, 0x63,

    /* U+0035 "5" */
    0x79, 0x4, 0x1e, 0xc, 0x30, 0xde,

    /* U+0036 "6" */
    0x76, 0x31, 0xed, 0xef, 0x6e,

    /* U+0037 "7" */
    0xf8, 0x8c, 0x66, 0x31, 0x8c,

    /* U+0038 "8" */
    0x73, 0x2c, 0x9e, 0x5b, 0x3c, 0xde,

    /* U+0039 "9" */
    0x76, 0xf7, 0xb7, 0x8c, 0x6e,

    /* U+003A ":" */
    0xf0, 0xf0,

    /* U+003B ";" */
    0xf0, 0xf6,

    /* U+003C "<" */
    0x1, 0xf9, 0xc3, 0x80,

    /* U+003D "=" */
    0xf8, 0x1, 0xf0,

    /* U+003E ">" */
    0x7, 0xe, 0x7e, 0x0,

    /* U+003F "?" */
    0xe3, 0x36, 0x40, 0x66,

    /* U+0040 "@" */
    0x1e, 0x30, 0x90, 0x33, 0x9a, 0x4d, 0x6e, 0xfd,
    0x0, 0x40, 0x1e, 0x0,

    /* U+0041 "A" */
    0x30, 0xe3, 0x9a, 0x49, 0xfc, 0xf1,

    /* U+0042 "B" */
    0xf3, 0x6d, 0xbc, 0xcf, 0x3c, 0xfe,

    /* U+0043 "C" */
    0x3d, 0x9c, 0x30, 0xc3, 0x6, 0x4f,

    /* U+0044 "D" */
    0xf3, 0x6c, 0xf3, 0xcf, 0x3d, 0xbc,

    /* U+0045 "E" */
    0xfe, 0x31, 0xfc, 0x63, 0x1f,

    /* U+0046 "F" */
    0xfe, 0x31, 0x8f, 0xe3, 0x18,

    /* U+0047 "G" */
    0x39, 0xac, 0x30, 0xdf, 0x36, 0xcf,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xff, 0xcf, 0x3c, 0xf3,

    /* U+0049 "I" */
    0xff, 0xff,

    /* U+004A "J" */
    0x18, 0xc6, 0x31, 0x8e, 0x7e,

    /* U+004B "K" */
    0xcd, 0xb3, 0x47, 0x8f, 0x99, 0x33, 0x62,

    /* U+004C "L" */
    0xc6, 0x31, 0x8c, 0x63, 0x1f,

    /* U+004D "M" */
    0xc7, 0xdf, 0xbf, 0x7e, 0xfa, 0xf5, 0xe3,

    /* U+004E "N" */
    0xcf, 0xbe, 0xfb, 0xdf, 0x7d, 0xf3,

    /* U+004F "O" */
    0x38, 0xdb, 0x1e, 0x3c, 0x78, 0xdb, 0x1c,

    /* U+0050 "P" */
    0xfb, 0x3c, 0xf3, 0xfb, 0xc, 0x30,

    /* U+0051 "Q" */
    0x38, 0xdb, 0x1e, 0x3c, 0x78, 0xdb, 0x1e, 0x18,
    0x1c,

    /* U+0052 "R" */
    0xfb, 0x3c, 0xf3, 0xfb, 0x6c, 0xb3,

    /* U+0053 "S" */
    0x76, 0xb1, 0xe7, 0x8e, 0x7e,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0xc,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xde,

    /* U+0056 "V" */
    0xc7, 0x34, 0xd2, 0x69, 0xe3, 0xc,

    /* U+0057 "W" */
    0xc9, 0xe6, 0xd7, 0x6a, 0xa5, 0x52, 0xa9, 0xdc,
    0x66,

    /* U+0058 "X" */
    0x4d, 0xa3, 0x8c, 0x31, 0xe4, 0xb3,

    /* U+0059 "Y" */
    0xcf, 0x34, 0x9e, 0x30, 0xc3, 0xc,

    /* U+005A "Z" */
    0xf8, 0xcc, 0x46, 0x23, 0x1f,

    /* U+005B "[" */
    0xea, 0xaa, 0xb0,

    /* U+005C "\\" */
    0x84, 0x44, 0x42, 0x22, 0x21, 0x10,

    /* U+005D "]" */
    0xd5, 0x55, 0x70,

    /* U+005E "^" */
    0x66, 0x69, 0x90,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0x19, 0x80,

    /* U+0061 "a" */
    0x70, 0xdf, 0xbd, 0xfc,

    /* U+0062 "b" */
    0xc6, 0x3d, 0xbd, 0xef, 0x7e,

    /* U+0063 "c" */
    0x7e, 0x31, 0x8c, 0xbc,

    /* U+0064 "d" */
    0x18, 0xdf, 0xbd, 0xef, 0x6f,

    /* U+0065 "e" */
    0x76, 0x7f, 0x8c, 0x3c,

    /* U+0066 "f" */
    0xfb, 0xed, 0xb6,

    /* U+0067 "g" */
    0x7f, 0x6d, 0x9c, 0xc1, 0xf8, 0xfe,

    /* U+0068 "h" */
    0xc6, 0x3f, 0xbd, 0xef, 0x7b,

    /* U+0069 "i" */
    0xcf, 0xff,

    /* U+006A "j" */
    0x61, 0xb6, 0xdb, 0x78,

    /* U+006B "k" */
    0xc3, 0xc, 0xb4, 0xf3, 0xcd, 0xb2,

    /* U+006C "l" */
    0xdb, 0x6d, 0xb7,

    /* U+006D "m" */
    0xff, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb,

    /* U+006E "n" */
    0xfe, 0xf7, 0xbd, 0xec,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+0070 "p" */
    0xf6, 0xf7, 0xbd, 0xfb, 0x18,

    /* U+0071 "q" */
    0x7e, 0xf7, 0xbd, 0xbc, 0x63,

    /* U+0072 "r" */
    0xfc, 0xcc, 0xcc,

    /* U+0073 "s" */
    0x78, 0xe7, 0x9e,

    /* U+0074 "t" */
    0x66, 0xf6, 0x66, 0x63,

    /* U+0075 "u" */
    0xde, 0xf7, 0xbd, 0xfc,

    /* U+0076 "v" */
    0xc9, 0x24, 0x9e, 0x30, 0xc0,

    /* U+0077 "w" */
    0xc9, 0x59, 0x5f, 0x57, 0x76, 0x36,

    /* U+0078 "x" */
    0x4b, 0xcc, 0x65, 0xe4,

    /* U+0079 "y" */
    0xc9, 0x25, 0x9c, 0x30, 0xc2, 0x18,

    /* U+007A "z" */
    0xf3, 0x64, 0xcf,

    /* U+007B "{" */
    0x69, 0x25, 0x12, 0x4c,

    /* U+007C "|" */
    0xff, 0xf0,

    /* U+007D "}" */
    0xc9, 0x24, 0x52, 0x58,

    /* U+007E "~" */
    0x60, 0x60,

    /* U+00D7 "×" */
    0x85, 0x23, 0xc, 0x4a, 0x10,

    /* U+00F7 "÷" */
    0x18, 0x30, 0x7, 0xf0, 0x3, 0x6, 0x0,

    /* U+201C "“" */
    0x5a, 0xf0,

    /* U+201D "”" */
    0xf5, 0xa0,

    /* U+2190 "←" */
    0x10, 0x8, 0xf, 0xfc, 0x80, 0x10, 0x0,

    /* U+2191 "↑" */
    0x23, 0xbe, 0x42, 0x10, 0x84, 0x20,

    /* U+2192 "→" */
    0x2, 0x0, 0x4f, 0xfc, 0x4, 0x2, 0x0,

    /* U+2193 "↓" */
    0x21, 0x8, 0x42, 0x13, 0xee, 0x20,

    /* U+221A "√" */
    0x7, 0xc1, 0x0, 0x80, 0x20, 0x8, 0x4, 0x5,
    0x1, 0x40, 0x20, 0x8, 0x0,

    /* U+3001 "、" */
    0xc6, 0x20,

    /* U+3002 "。" */
    0x69, 0x96,

    /* U+300A "《" */
    0x1, 0x94, 0xaa, 0x51, 0x8a, 0x31, 0x40,

    /* U+300B "》" */
    0x3, 0x14, 0xa2, 0x94, 0xca, 0x65, 0x0,

    /* U+4E00 "一" */
    0xff, 0x80,

    /* U+4E08 "丈" */
    0x4, 0x7f, 0xd1, 0xc, 0x82, 0xc0, 0xc0, 0xf1,
    0xcf,

    /* U+4E2A "个" */
    0x8, 0xe, 0xd, 0x9d, 0x30, 0x80, 0x40, 0x20,
    0x10, 0x8, 0x0,

    /* U+4E3B "主" */
    0x18, 0x6, 0x3f, 0xe1, 0x0, 0x83, 0xf8, 0x20,
    0x10, 0xff, 0x80,

    /* U+4EB2 "亲" */
    0x8, 0x3f, 0xc8, 0x9f, 0xf0, 0x87, 0xfc, 0xa8,
    0x92, 0x19, 0x0,

    /* U+4ECE "从" */
    0x22, 0x11, 0x8, 0x84, 0x43, 0x63, 0xf9, 0x14,
    0x9b, 0xd8, 0x80,

    /* U+4F2F "伯" */
    0x24, 0x64, 0x5f, 0xd1, 0xd1, 0x5f, 0x51, 0x51,
    0x5f, 0x51,

    /* U+4F84 "侄" */
    0x7f, 0xa4, 0xb6, 0x5f, 0xf4, 0x43, 0xfd, 0x10,
    0xff, 0x40, 0x0,

    /* U+513F "儿" */
    0x22, 0x11, 0x8, 0x84, 0x42, 0x21, 0x11, 0x8a,
    0x85, 0x83, 0x80,

    /* U+5144 "兄" */
    0x7f, 0x20, 0x90, 0x4f, 0xe3, 0x41, 0x20, 0x92,
    0xc9, 0xc7, 0x80,

    /* U+5165 "入" */
    0x10, 0x2, 0x0, 0xc0, 0x38, 0x1a, 0x6, 0xc3,
    0x19, 0x83, 0x40, 0x0,

    /* U+516C "公" */
    0x12, 0xc, 0xc6, 0x19, 0x22, 0x18, 0x4, 0xc2,
    0x11, 0xfe, 0x0, 0x80,

    /* U+5173 "关" */
    0x63, 0x19, 0x1f, 0xc1, 0xf, 0xf8, 0xe0, 0xd9,
    0xc7, 0x0, 0x80,

    /* U+517B "养" */
    0x22, 0x7f, 0xdf, 0xdf, 0xf6, 0x23, 0x1b, 0xde,
    0x4c, 0x46, 0x0,

    /* U+518D "再" */
    0xff, 0x84, 0x1f, 0xc9, 0x27, 0xf2, 0x4b, 0xfe,
    0x82, 0x47, 0x0,

    /* U+52A9 "助" */
    0x72, 0x29, 0x1d, 0xea, 0x57, 0x2a, 0x95, 0xf3,
    0xd9, 0xb, 0x80,

    /* U+5355 "单" */
    0x22, 0x3f, 0x92, 0x4f, 0xe4, 0x91, 0xf3, 0xfe,
    0x10, 0x8, 0x0,

    /* U+53D4 "叔" */
    0x27, 0x9f, 0xc9, 0x3f, 0xf2, 0x37, 0x5a, 0xad,
    0x4f, 0x64, 0x80,

    /* U+54E5 "哥" */
    0xff, 0xbe, 0x91, 0x4f, 0xaf, 0xfb, 0xe9, 0x14,
    0xfa, 0x3, 0x0,

    /* U+5668 "器" */
    0x77, 0x2a, 0x9d, 0xc3, 0x4f, 0xfb, 0x1b, 0xde,
    0xaa, 0x77, 0x0,

    /* U+56DE "回" */
    0xff, 0x81, 0xbd, 0xa5, 0xa5, 0xbd, 0x81, 0xff,
    0x81,

    /* U+5802 "堂" */
    0x49, 0x7f, 0xe0, 0x37, 0xd2, 0x20, 0xe1, 0xfc,
    0x10, 0xff, 0x80,

    /* U+5916 "外" */
    0x22, 0xf, 0x84, 0xb1, 0xac, 0x3a, 0x86, 0x83,
    0x21, 0x88, 0x42, 0x0,

    /* U+592B "夫" */
    0x8, 0x3f, 0x82, 0x1, 0xf, 0xf8, 0xe0, 0xd0,
    0xc6, 0xc1, 0x80,

    /* U+5973 "女" */
    0x18, 0x8, 0x3f, 0xe4, 0x46, 0x63, 0xa0, 0x39,
    0xf7, 0xc1, 0x0,

    /* U+5976 "奶" */
    0x4f, 0x7a, 0xbd, 0x5a, 0xff, 0x4b, 0x64, 0xe2,
    0xd1, 0xdb, 0x80,

    /* U+59AF "妯" */
    0x42, 0x21, 0x3f, 0xf7, 0x5b, 0xfb, 0xd5, 0xeb,
    0xff, 0x98, 0x80,

    /* U+59B9 "妹" */
    0x42, 0x77, 0xe8, 0x97, 0xfe, 0x73, 0x39, 0xbf,
    0xf5, 0x82, 0x0,

    /* U+59BB "妻" */
    0xff, 0xbf, 0x9f, 0xdf, 0xf7, 0xf7, 0xfc, 0xc8,
    0x3c, 0xfb, 0x0,

    /* U+59D0 "姐" */
    0x4f, 0x24, 0xba, 0x55, 0xea, 0x97, 0x79, 0x24,
    0xd2, 0x9f, 0x80,

    /* U+59D1 "姑" */
    0x42, 0x21, 0x3f, 0xfa, 0x4f, 0xfb, 0x65, 0xf3,
    0xbf, 0x8c, 0x80,

    /* U+59E5 "姥" */
    0x44, 0x2f, 0xb9, 0xd7, 0xfa, 0xc5, 0xf9, 0x70,
    0xd9, 0xa7, 0x0,

    /* U+59E8 "姨" */
    0x5f, 0xb9, 0x6, 0x7a, 0xbe, 0xaf, 0xb9, 0x26,
    0xfb, 0xac, 0x90, 0x80,

    /* U+5A0C "娌" */
    0x5f, 0xad, 0x7f, 0xf7, 0x5b, 0xfd, 0x11, 0x7e,
    0xc4, 0xbf, 0x80,

    /* U+5A7F "婿" */
    0x5f, 0xa5, 0x7a, 0xf7, 0xfa, 0xff, 0x7d, 0xbf,
    0xd1, 0x89, 0x80,

    /* U+5AB3 "媳" */
    0x44, 0x27, 0xbb, 0xd5, 0xea, 0xf7, 0x21, 0xec,
    0xf3, 0x9e, 0x0,

    /* U+5AE1 "嫡" */
    0x42, 0x76, 0xeb, 0xf5, 0x5a, 0xaa, 0x7d, 0xff,
    0xd1, 0x89, 0x80,

    /* U+5B50 "子" */
    0x7f, 0x3, 0x3, 0x1, 0xf, 0xf8, 0x40, 0x20,
    0x10, 0x38, 0x0,

    /* U+5B59 "孙" */
    0xf2, 0x11, 0x13, 0xe9, 0x5e, 0xae, 0xd5, 0x8,
    0x84, 0x46, 0x0,

    /* U+5B9A "定" */
    0x8, 0xff, 0x81, 0x7e, 0x68, 0x4f, 0x68, 0xf8,
    0x9f,

    /* U+5C5E "属" */
    0x7f, 0x41, 0x7f, 0x5f, 0x5f, 0x5f, 0x7f, 0xff,
    0xb1,

    /* U+5CB3 "岳" */
    0x0, 0x3f, 0x98, 0xf, 0xe6, 0x27, 0xfc, 0x20,
    0x92, 0x49, 0x3f, 0x80,

    /* U+5DF1 "己" */
    0xfe, 0x2, 0x2, 0xfe, 0x82, 0x80, 0x81, 0x81,
    0xff,

    /* U+5E2E "帮" */
    0xff, 0x92, 0xbf, 0x5f, 0x96, 0x53, 0xf9, 0x24,
    0x96, 0x8, 0x0,

    /* U+5EB6 "庶" */
    0x4, 0x1f, 0xe4, 0x91, 0xfe, 0x49, 0x13, 0xc5,
    0x9, 0xda, 0x26, 0x80,

    /* U+5F1F "弟" */
    0x22, 0x7f, 0x82, 0x4f, 0xe4, 0x83, 0xfc, 0xe7,
    0xd6, 0x8, 0x0,

    /* U+62E9 "择" */
    0x5f, 0x77, 0x13, 0x8b, 0x6e, 0x42, 0x71, 0x7c,
    0x88, 0xc4, 0x0,

    /* U+6301 "持" */
    0x42, 0x27, 0xf8, 0x8b, 0xf6, 0x17, 0xfd, 0x24,
    0x8a, 0xc3, 0x0,

    /* U+652F "支" */
    0x8, 0x7f, 0xc2, 0xf, 0xe6, 0x31, 0xb0, 0x71,
    0xef, 0x0, 0x0,

    /* U+660E "明" */
    0xef, 0xa9, 0xa9, 0xef, 0xa9, 0xef, 0x89, 0x11,
    0x13,

    /* U+662F "是" */
    0x7f, 0x3f, 0x90, 0x4f, 0xef, 0xf9, 0x41, 0xbe,
    0xf0, 0x9f, 0x80,

    /* U+663E "显" */
    0x7f, 0x20, 0x9f, 0xcf, 0xe5, 0x52, 0xa8, 0x51,
    0xff,

    /* U+66FE "曾" */
    0x64, 0xff, 0xd5, 0x91, 0xff, 0x7e, 0x7e, 0x7e,
    0x42,

    /* U+672A "未" */
    0x8, 0x3f, 0x82, 0x1, 0xf, 0xf8, 0xe0, 0xa9,
    0x93, 0x88, 0x0,

    /* U+679C "果" */
    0x7f, 0x24, 0x9f, 0xc9, 0x23, 0xe7, 0xfc, 0xf9,
    0xd7, 0x88, 0x80,

    /* U+67E5 "查" */
    0x8, 0x7f, 0xcf, 0x8d, 0x6f, 0xeb, 0xf1, 0x88,
    0x38, 0xff, 0x80,

    /* U+683C "格" */
    0x4f, 0x7e, 0x99, 0x9f, 0xfd, 0x8e, 0x79, 0x24,
    0x9e, 0x49, 0x0,

    /* U+6BCD "母" */
    0x3f, 0x34, 0x9b, 0x5f, 0xf5, 0x92, 0x49, 0xfe,
    0x4, 0x6, 0x0,

    /* U+6E05 "清" */
    0x7f, 0xc3, 0xe4, 0x21, 0xff, 0xf, 0x8b, 0xe2,
    0xf9, 0x22, 0x49, 0x80,

    /* U+7236 "父" */
    0x33, 0x8, 0x65, 0x28, 0x48, 0x1e, 0x3, 0x1,
    0xe1, 0xce, 0x40, 0x80,

    /* U+7237 "爷" */
    0x12, 0x39, 0xd7, 0x83, 0xc6, 0x1b, 0xf8, 0x24,
    0x12, 0xb, 0x4, 0x0,

    /* U+7384 "玄" */
    0x8, 0x7f, 0xc4, 0x4, 0x47, 0xc0, 0xd0, 0xc4,
    0xff, 0x0, 0x80,

    /* U+7525 "甥" */
    0x6f, 0xbf, 0xea, 0xa5, 0xf7, 0x21, 0x7c, 0xca,
    0xc9, 0x9, 0x80,

    /* U+754C "界" */
    0x7f, 0x24, 0x9f, 0xcf, 0xe3, 0x63, 0x1b, 0xde,
    0x6c, 0x66, 0x0,

    /* U+7684 "的" */
    0x44, 0x32, 0x3d, 0xf3, 0x99, 0x8f, 0xb6, 0x4b,
    0x21, 0xf0, 0xc1, 0x80,

    /* U+77E5 "知" */
    0x40, 0xff, 0xad, 0x2d, 0xfd, 0x6d, 0x7d, 0xdf,
    0x80,

    /* U+786E "确" */
    0xf7, 0x24, 0x93, 0xff, 0x5d, 0xfe, 0xd5, 0x7e,
    0xf5, 0x9, 0x80,

    /* U+793A "示" */
    0x7f, 0x0, 0x0, 0x1f, 0xf0, 0x83, 0x59, 0x25,
    0x13, 0x38, 0x0,

    /* U+7956 "祖" */
    0x40, 0x27, 0xba, 0x5d, 0x26, 0xf7, 0x4b, 0xbc,
    0x92, 0x49, 0x2f, 0xc0,

    /* U+7A7A "空" */
    0x8, 0x7f, 0xe0, 0x3e, 0xf7, 0xf0, 0x40, 0x21,
    0xff,

    /* U+7B97 "算" */
    0x77, 0xd5, 0x9f, 0xcf, 0xe7, 0xf3, 0xfb, 0xfe,
    0xc4, 0x42, 0x0,

    /* U+7CFB "系" */
    0x7, 0x3e, 0xd, 0x8f, 0x81, 0xa1, 0x99, 0xf6,
    0x54, 0x49, 0x4c, 0x0,

    /* U+7ED3 "结" */
    0x22, 0x37, 0xf4, 0x9d, 0xf6, 0x7, 0xfc, 0x37,
    0xff, 0xd, 0x80,

    /* U+7EE7 "继" */
    0x29, 0x13, 0x65, 0x9b, 0xbf, 0x6b, 0xbf, 0xd1,
    0x93, 0xbf,

    /* U+81EA "自" */
    0x11, 0xfe, 0xf, 0xf8, 0x3f, 0xe0, 0xff, 0x82,

    /* U+8205 "舅" */
    0x77, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe,
    0x31, 0x63, 0x80,

    /* U+83DC "菜" */
    0xff, 0x91, 0x3f, 0x89, 0x26, 0x97, 0xfc, 0xf9,
    0xd7, 0x88, 0x80,

    /* U+8868 "表" */
    0xff, 0x84, 0x1f, 0xdf, 0xf3, 0x57, 0x3a, 0x8c,
    0xf3, 0x0, 0x0,

    /* U+8BA1 "计" */
    0x44, 0x64, 0x4, 0xdf, 0x44, 0x44, 0x64, 0x64,
    0x44,

    /* U+8BE2 "询" */
    0x8, 0x2f, 0xc4, 0x3b, 0xd5, 0x2a, 0xf5, 0xfa,
    0xa3, 0x3, 0x0,

    /* U+8BF4 "说" */
    0x19, 0x25, 0x7, 0xda, 0x25, 0xf2, 0x51, 0xa8,
    0xf5, 0x33, 0x80,

    /* U+8BF7 "请" */
    0xdf, 0xf, 0xb7, 0xfb, 0xf5, 0xf2, 0x89, 0xfc,
    0xfe, 0x13, 0x0,

    /* U+8F93 "输" */
    0x46, 0x75, 0xa7, 0xf8, 0xf, 0xfa, 0xff, 0xfe,
    0xa9, 0x55, 0x80,

    /* U+8FD4 "返" */
    0x1, 0x6f, 0x94, 0x3, 0xed, 0x12, 0xb9, 0x5c,
    0xfa, 0xa8, 0x4f, 0xc0,

    /* U+8FD9 "这" */
    0x4, 0x62, 0x1f, 0xe2, 0x4d, 0xe2, 0x31, 0x6c,
    0xf2, 0xa0, 0x4f, 0xc0,

    /* U+9009 "选" */
    0x94, 0x2f, 0x81, 0x1f, 0xe4, 0xa2, 0xd5, 0xcd,
    0xc0, 0x9f, 0x0,

    /* U+9664 "除" */
    0x4, 0x72, 0x3a, 0xda, 0x3c, 0xf5, 0x22, 0xff,
    0xfa, 0x95, 0xc6, 0x0,

    /* U+9762 "面" */
    0xff, 0x84, 0x3f, 0xfa, 0xbd, 0xde, 0xaf, 0x77,
    0xff, 0xc1, 0x80,

    /* U+FF01 "！" */
    0xff, 0xcf,

    /* U+FF0C "，" */
    0xf6,

    /* U+FF1A "：" */
    0xf0, 0x3c,

    /* U+FF1B "；" */
    0xf0, 0x3d, 0x80,

    /* U+FF1F "？" */
    0xf1, 0x12, 0x66, 0x60
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 36, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 59, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 92, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 5, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 10, .adv_w = 94, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 17, .adv_w = 154, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 26, .adv_w = 118, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 52, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 34, .adv_w = 60, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 37, .adv_w = 60, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 40, .adv_w = 81, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 43, .adv_w = 94, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 47, .adv_w = 52, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 48, .adv_w = 59, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 49, .adv_w = 52, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 62, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 56, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 94, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 70, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 94, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 94, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 52, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 52, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 111, .adv_w = 94, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 115, .adv_w = 94, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 118, .adv_w = 94, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 122, .adv_w = 82, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 161, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 138, .adv_w = 103, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 109, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 105, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 114, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 162, .adv_w = 98, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 121, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 53, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 91, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 110, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 92, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 137, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 216, .adv_w = 123, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 107, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 123, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 238, .adv_w = 109, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 100, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 100, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 146, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 100, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 93, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 98, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 60, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 296, .adv_w = 62, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 302, .adv_w = 60, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 305, .adv_w = 94, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 308, .adv_w = 91, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 309, .adv_w = 100, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 311, .adv_w = 95, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 103, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 320, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 103, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 93, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 60, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 342, .adv_w = 102, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 49, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 349, .adv_w = 49, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 353, .adv_w = 97, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 359, .adv_w = 50, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 154, .box_w = 8, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 368, .adv_w = 103, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 372, .adv_w = 100, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 103, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 382, .adv_w = 103, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 387, .adv_w = 70, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 79, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 393, .adv_w = 67, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 102, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 401, .adv_w = 92, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 138, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 90, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 92, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 422, .adv_w = 82, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 425, .adv_w = 60, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 429, .adv_w = 47, .box_w = 1, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 431, .adv_w = 60, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 435, .adv_w = 94, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 437, .adv_w = 160, .box_w = 6, .box_h = 6, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 442, .adv_w = 160, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 449, .adv_w = 160, .box_w = 4, .box_h = 3, .ofs_x = 6, .ofs_y = 6},
    {.bitmap_index = 451, .adv_w = 160, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 453, .adv_w = 160, .box_w = 10, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 460, .adv_w = 160, .box_w = 5, .box_h = 9, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 466, .adv_w = 160, .box_w = 10, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 473, .adv_w = 160, .box_w = 5, .box_h = 9, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 479, .adv_w = 160, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 492, .adv_w = 160, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 494, .adv_w = 160, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 496, .adv_w = 160, .box_w = 5, .box_h = 10, .ofs_x = 5, .ofs_y = -1},
    {.bitmap_index = 503, .adv_w = 160, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 510, .adv_w = 160, .box_w = 9, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 512, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 521, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 532, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 543, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 554, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 565, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 575, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 586, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 597, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 608, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 620, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 632, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 643, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 654, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 665, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 676, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 687, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 698, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 709, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 720, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 729, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 740, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 752, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 763, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 774, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 785, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 796, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 807, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 818, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 829, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 840, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 851, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 863, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 874, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 885, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 896, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 907, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 918, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 929, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 938, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 947, .adv_w = 160, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 959, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 968, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 979, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 991, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1002, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1013, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1024, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1035, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1044, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1055, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1064, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1073, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1084, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1095, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1106, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1117, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1128, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1140, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1152, .adv_w = 160, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1164, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1175, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1186, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1197, .adv_w = 160, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1209, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1218, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1229, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1240, .adv_w = 160, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1252, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1261, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1272, .adv_w = 160, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1284, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1295, .adv_w = 160, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1305, .adv_w = 160, .box_w = 7, .box_h = 9, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 1313, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1324, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1335, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1346, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1355, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1366, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1377, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1388, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1399, .adv_w = 160, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1411, .adv_w = 160, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1423, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1434, .adv_w = 160, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1446, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1457, .adv_w = 160, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1459, .adv_w = 160, .box_w = 2, .box_h = 4, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 1460, .adv_w = 160, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1462, .adv_w = 160, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1465, .adv_w = 160, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x20, 0x1f45, 0x1f46, 0x20b9, 0x20ba, 0x20bb, 0x20bc,
    0x2143, 0x2f2a, 0x2f2b, 0x2f33, 0x2f34, 0x4d29, 0x4d31, 0x4d53,
    0x4d64, 0x4ddb, 0x4df7, 0x4e58, 0x4ead, 0x5068, 0x506d, 0x508e,
    0x5095, 0x509c, 0x50a4, 0x50b6, 0x51d2, 0x527e, 0x52fd, 0x540e,
    0x5591, 0x5607, 0x572b, 0x583f, 0x5854, 0x589c, 0x589f, 0x58d8,
    0x58e2, 0x58e4, 0x58f9, 0x58fa, 0x590e, 0x5911, 0x5935, 0x59a8,
    0x59dc, 0x5a0a, 0x5a79, 0x5a82, 0x5ac3, 0x5b87, 0x5bdc, 0x5d1a,
    0x5d57, 0x5ddf, 0x5e48, 0x6212, 0x622a, 0x6458, 0x6537, 0x6558,
    0x6567, 0x6627, 0x6653, 0x66c5, 0x670e, 0x6765, 0x6af6, 0x6d2e,
    0x715f, 0x7160, 0x72ad, 0x744e, 0x7475, 0x75ad, 0x770e, 0x7797,
    0x7863, 0x787f, 0x79a3, 0x7ac0, 0x7c24, 0x7dfc, 0x7e10, 0x8113,
    0x812e, 0x8305, 0x8791, 0x8aca, 0x8b0b, 0x8b1d, 0x8b20, 0x8ebc,
    0x8efd, 0x8f02, 0x8f32, 0x958d, 0x968b, 0xfe2a, 0xfe35, 0xfe43,
    0xfe44, 0xfe48
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 215, .range_length = 65097, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 106, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
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
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
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
const lv_font_t lv_font_siyuan_10 = {
#else
lv_font_t lv_font_siyuan_10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 13,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
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



#endif /*#if LV_FONT_SIYUAN_10*/

