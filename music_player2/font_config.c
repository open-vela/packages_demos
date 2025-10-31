#include "font_config.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if LV_USE_FREETYPE
lv_font_t* misans_font_16 = NULL;
lv_font_t* misans_font_20 = NULL;
lv_font_t* misans_font_24 = NULL;
lv_font_t* misans_font_28 = NULL;
lv_font_t* misans_font_32 = NULL;
#endif

static bool font_system_initialized = false;

const lv_font_t* get_font_by_size(int size) {
#if LV_USE_FREETYPE
    if (size >= 32 && CHINESE_FONT_32) {
        return CHINESE_FONT_32;
    } else if (size >= 28 && CHINESE_FONT_28) {
        return CHINESE_FONT_28;
    } else if (size >= 24 && CHINESE_FONT_24) {
        return CHINESE_FONT_24;
    } else if (size >= 20 && CHINESE_FONT_20) {
        return CHINESE_FONT_20;
    } else if (CHINESE_FONT_16) {
        return CHINESE_FONT_16;
    }
#endif

    if (size >= 32) {
        return FONT_DEFAULT_32;
    } else if (size >= 28) {
        return FONT_DEFAULT_28;
    } else if (size >= 24) {
        return FONT_DEFAULT_24;
    } else if (size >= 20) {
        return FONT_DEFAULT_20;
    } else {
        return FONT_DEFAULT_16;
    }
}
void set_label_utf8_text(lv_obj_t* label, const char* text,
    const lv_font_t* font) {
    if (!label || !text)
        return;

    if (font) {
        lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    }

    lv_label_set_text(label, text);
}
const lv_font_t* get_playlist_font(const char* font_type) {
    if (!font_type)
        return get_font_by_size(16);

    if (strcmp(font_type, "title") == 0) {
        return get_font_by_size(PLAYLIST_TITLE_FONT_SIZE);
    } else if (strcmp(font_type, "song") == 0) {
        return get_font_by_size(PLAYLIST_SONG_NAME_FONT_SIZE);
    } else if (strcmp(font_type, "artist") == 0) {
        return get_font_by_size(PLAYLIST_ARTIST_FONT_SIZE);
    } else if (strcmp(font_type, "info") == 0) {
        return get_font_by_size(PLAYLIST_INFO_FONT_SIZE);
    } else if (strcmp(font_type, "button") == 0) {
        return get_font_by_size(PLAYLIST_BUTTON_FONT_SIZE);
    }

    return get_font_by_size(16);
}
void optimize_text_display(lv_obj_t* label, const char* text,
    const char* font_type) {
    if (!label || !text)
        return;

    const lv_font_t* font = get_playlist_font(font_type);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_label_set_text(label, text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);

    if (strcmp(font_type, "title") == 0) {
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    } else if (strcmp(font_type, "song") == 0) {
        lv_obj_set_style_text_color(label, lv_color_hex(0xF3F4F6), LV_PART_MAIN);
    } else if (strcmp(font_type, "artist") == 0) {
        lv_obj_set_style_text_color(label, lv_color_hex(0x9CA3AF), LV_PART_MAIN);
    } else if (strcmp(font_type, "info") == 0) {
        lv_obj_set_style_text_color(label, lv_color_hex(0x6B7280), LV_PART_MAIN);
    }
}
int font_system_init(void) {
    font_system_initialized = true;
    return 0;
}
