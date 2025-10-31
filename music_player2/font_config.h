#ifndef FONT_CONFIG_H
#define FONT_CONFIG_H

#include "lvgl.h"

#define FONT_UTF8_SUPPORT 1
#define FONT_CHINESE_SUPPORT 1

#define PLAYLIST_TITLE_FONT_SIZE 32
#define PLAYLIST_SONG_NAME_FONT_SIZE 24
#define PLAYLIST_ARTIST_FONT_SIZE 20
#define PLAYLIST_INFO_FONT_SIZE 16
#define PLAYLIST_BUTTON_FONT_SIZE 18

#define MISANS_NORMAL_PATH "/data/res/fonts/MiSans-Normal.ttf"
#define MISANS_SEMIBOLD_PATH "/data/res/fonts/MiSans-Semibold.ttf"

#if LV_FONT_MONTSERRAT_16
#define FONT_DEFAULT_16 &lv_font_montserrat_16
#else
#define FONT_DEFAULT_16 &lv_font_default
#endif

#if LV_FONT_MONTSERRAT_20
#define FONT_DEFAULT_20 &lv_font_montserrat_20
#else
#define FONT_DEFAULT_20 FONT_DEFAULT_16
#endif

#if LV_FONT_MONTSERRAT_24
#define FONT_DEFAULT_24 &lv_font_montserrat_24
#else
#define FONT_DEFAULT_24 FONT_DEFAULT_20
#endif

#if LV_FONT_MONTSERRAT_28
#define FONT_DEFAULT_28 &lv_font_montserrat_28
#else
#define FONT_DEFAULT_28 FONT_DEFAULT_24
#endif

#if LV_FONT_MONTSERRAT_32
#define FONT_DEFAULT_32 &lv_font_montserrat_32
#else
#define FONT_DEFAULT_32 FONT_DEFAULT_28
#endif

#if LV_USE_FREETYPE
extern lv_font_t* misans_font_16;
extern lv_font_t* misans_font_20;
extern lv_font_t* misans_font_24;
extern lv_font_t* misans_font_28;
extern lv_font_t* misans_font_32;

#define CHINESE_FONT_16 misans_font_16
#define CHINESE_FONT_20 misans_font_20
#define CHINESE_FONT_24 misans_font_24
#define CHINESE_FONT_28 misans_font_28
#define CHINESE_FONT_32 misans_font_32
#else
#define CHINESE_FONT_16 FONT_DEFAULT_16
#define CHINESE_FONT_20 FONT_DEFAULT_20
#define CHINESE_FONT_24 FONT_DEFAULT_24
#define CHINESE_FONT_28 FONT_DEFAULT_28
#define CHINESE_FONT_32 FONT_DEFAULT_32
#endif

const lv_font_t* get_font_by_size(int size);
void set_label_utf8_text(lv_obj_t* label, const char* text,
    const lv_font_t* font);
int font_system_init(void);
const lv_font_t* get_playlist_font(const char* font_type);
void optimize_text_display(lv_obj_t* label, const char* text,
    const char* font_type);

#endif
