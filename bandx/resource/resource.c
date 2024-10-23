/**
 * @file resource.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "resource.h"

/*********************
 *      DEFINES
 *********************/

#define ARRAY_SIZE(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

/* import images*/
#define IMG_DEF(NAME) LV_IMG_DECLARE(img_src_##NAME);
#include "image/img_src.inc"
#undef IMG_DEF

#define FONT_BASE_PATH "/font/"
#define IMG_BASE_PATH "/image/"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    const char* key;
    const char* img_src;
} resource_img_t;

typedef struct {
    const char* key;
    const lv_font_t* font;
} resource_font_t;

typedef struct {
    const char* key;
    lv_style_t style;
} resource_style_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

#define IMG_DEF(NAME) { #NAME, CONFIG_BANDX_BASE_PATH IMG_BASE_PATH #NAME ".png" },
static const resource_img_t g_img_resource_map[] = {
#include "image/img_src.inc"
};
#undef IMG_DEF

#define FONT_DEF(NAME, SIZE) { "default", LV_FONT_DEFAULT },
static resource_font_t g_font_resource_map[] = {
#include "font/font.inc"
};
#undef FONT_DEF

#define STYLE_DEF(NAME) { "empty" },
static resource_style_t g_style_resource_map[] = {
#include "style/style.inc"
};
#undef STYLE_DEF

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void resource_init(void)
{
    /* create fonts */
    int font_index = 0;
#define FONT_DEF(NAME, SIZE)                                                                          \
    do {                                                                                              \
        lv_font_t* font = lv_freetype_font_create(CONFIG_BANDX_BASE_PATH FONT_BASE_PATH #NAME ".ttf", \
            LV_FREETYPE_FONT_RENDER_MODE_BITMAP,                                                      \
            SIZE,                                                                                     \
            LV_FREETYPE_FONT_STYLE_NORMAL);                                                           \
        if (font) {                                                                                   \
            g_font_resource_map[font_index].key = #NAME "_" #SIZE;                                    \
            g_font_resource_map[font_index].font = font;                                              \
            font_index++;                                                                             \
        } else {                                                                                      \
            LV_LOG_WARN("font_" #NAME "_" #SIZE "not found");                                         \
        }                                                                                             \
    } while (0);
#include "font/font.inc"
#undef FONT_DEF
    LV_LOG_USER("create %d fonts", font_index);

    /* init styles */
    int style_index = 0;
#define STYLE_DEF(NAME)                                                \
    do {                                                               \
        extern void style_##NAME##_init(lv_style_t* style);            \
        g_style_resource_map[style_index].key = #NAME;                 \
        lv_style_init(&g_style_resource_map[style_index].style);       \
        style_##NAME##_init(&g_style_resource_map[style_index].style); \
        style_index++;                                                 \
    } while (0);
#include "style/style.inc"
#undef STYLE_DEF
    LV_LOG_USER("create %d styles", style_index);
}

const lv_font_t* resource_get_font(const char* key)
{
    for (int i = 0; i < ARRAY_SIZE(g_font_resource_map); i++) {
        if (lv_strcmp(key, g_font_resource_map[i].key) == 0) {
            return g_font_resource_map[i].font;
        }
    }

    LV_LOG_WARN("key %s not found", key);
    return LV_FONT_DEFAULT;
}

const void* resource_get_img(const char* key)
{
    for (int i = 0; i < ARRAY_SIZE(g_img_resource_map); i++) {
        if (lv_strcmp(key, g_img_resource_map[i].key) == 0) {
            return g_img_resource_map[i].img_src;
        }
    }

    LV_LOG_WARN("key %s not found", key);
    return LV_SYMBOL_IMAGE;
}

lv_style_t* resource_get_style(const char* key)
{
    for (int i = 0; i < ARRAY_SIZE(g_style_resource_map); i++) {
        if (lv_strcmp(key, g_style_resource_map[i].key) == 0) {
            return &g_style_resource_map[i].style;
        }
    }

    LV_LOG_WARN("key %s not found", key);
    return &g_style_resource_map[0].style;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
