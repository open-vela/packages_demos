//
// Created by BenignX on 2024/3/21.
//

#ifndef LVGL_APP_H
#define LVGL_APP_H

#include "audio_ctl.h"
#include "lvgl.h"
#include "wifi.h"

#define RES_ROOT CONFIG_LVX_MUSIC_PLAYER_DATA_ROOT "/res"
#define FONTS_ROOT RES_ROOT "/fonts"
#define ICONS_ROOT RES_ROOT "/icons"
#define MUSICS_ROOT RES_ROOT "/musics"

typedef struct _album_info_t {
    const char* name;
    const char* artist;
    char path[LV_FS_MAX_PATH_LENGTH];
    char cover[LV_FS_MAX_PATH_LENGTH];
    uint64_t total_time; /**< in milliseconds */
    lv_color_t color;
} album_info_t;

typedef enum _switch_album_mode_t {
    SWITCH_ALBUM_MODE_PREV,
    SWITCH_ALBUM_MODE_NEXT,
} switch_album_mode_t;

typedef enum _play_status_t {
    PLAY_STATUS_STOP,
    PLAY_STATUS_PLAY,
    PLAY_STATUS_PAUSE,
} play_status_t;

struct resource_s {
    struct {
        lv_obj_t* time;
        lv_obj_t* date;

        lv_obj_t* player_group;

        lv_obj_t* volume_bar;
        lv_obj_t* volume_bar_indic;
        lv_obj_t* audio;
        lv_obj_t* playlist_base;

        lv_obj_t* album_cover;
        lv_obj_t* album_name;
        lv_obj_t* album_artist;

        lv_obj_t* play_btn;
        lv_obj_t* playback_group;
        lv_obj_t* playback_progress;
        lv_span_t* playback_current_time;
        lv_span_t* playback_total_time;

        lv_obj_t* playlist;
    } ui;

    struct {
        struct {
            lv_font_t* normal;
        } size_16;
        struct {
            lv_font_t* bold;
        } size_22;
        struct {
            lv_font_t* normal;
        } size_24;
        struct {
            lv_font_t* normal;
        } size_28;
        struct {
            lv_font_t* bold;
        } size_60;
    } fonts;

    struct {
        lv_style_t button_default;
        lv_style_t button_pressed;
        lv_style_transition_dsc_t button_transition_dsc;
        lv_style_transition_dsc_t transition_dsc;
    } styles;

    struct {
        const char* playlist;
        const char* previous;
        const char* play;
        const char* pause;
        const char* next;
        const char* audio;
        const char* mute;
        const char* music;
        const char* nocover;
    } images;

    album_info_t* albums;
    uint8_t album_count;
};

struct ctx_s {
    album_info_t* current_album;
    lv_obj_t* current_album_related_obj;

    uint16_t volume;

    play_status_t play_status_prev;
    play_status_t play_status;
    uint64_t current_time;

    struct {
        lv_timer_t* volume_bar_countdown;
        lv_timer_t* playback_progress_update;
    } timers;

    audioctl_s* audioctl;
};

struct conf_s {
#if WIFI_ENABLED
    wifi_conf_t wifi;
#endif
};

void app_create(void);

#endif // LVGL_APP_H
