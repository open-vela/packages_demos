//
// Vela Music Player - Font Configuration File
// Created by Vela on 2025/7/27
// Font configuration supporting UTF-8 encoding and Chinese display
//

#ifndef FONT_CONFIG_H
#define FONT_CONFIG_H

#include "lvgl.h"

/*********************
 *      FONT CONFIGURATION
 *********************/

// Font encoding support
#define FONT_UTF8_SUPPORT 1
#define FONT_CHINESE_SUPPORT 1

// Playlist specific font size definitions
#define PLAYLIST_TITLE_FONT_SIZE 32      // Playlist title font
#define PLAYLIST_SONG_NAME_FONT_SIZE 24  // Song name font
#define PLAYLIST_ARTIST_FONT_SIZE 20     // Artist font
#define PLAYLIST_INFO_FONT_SIZE 16       // Info font
#define PLAYLIST_BUTTON_FONT_SIZE 18     // Button font

// MiSans font paths
#define MISANS_NORMAL_PATH "/data/res/fonts/MiSans-Normal.ttf"
#define MISANS_SEMIBOLD_PATH "/data/res/fonts/MiSans-Semibold.ttf"

// Use system default fonts (supporting basic Latin characters) + runtime font rendering
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

// Chinese font support - if FreeType is supported
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

/*********************
 *      FONT FUNCTIONS
 *********************/

/**
 * Get font by size
 */
const lv_font_t* get_font_by_size(int size);


/**
 * Set UTF-8 text for label
 */
void set_label_utf8_text(lv_obj_t* label, const char* text, const lv_font_t* font);

/**
 * Initialize font system
 */
int font_system_init(void);

/**
 * Get dedicated font for playlist
 * @param font_type Font type: title, song, artist, info, button
 * @return Corresponding font pointer
 */
const lv_font_t* get_playlist_font(const char* font_type);

/**
 * Optimize text display effects
 * @param label Label object
 * @param text Text to display
 * @param font_type Font type
 */
void optimize_text_display(lv_obj_t* label, const char* text, const char* font_type);

#endif // FONT_CONFIG_H
