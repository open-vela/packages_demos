/**
 * Music Player Header
 * Core data structures and definitions for LVGL-based music player
 */

#ifndef LVGL_APP_H
#define LVGL_APP_H

#include "audio_ctl.h"
#include "lvgl.h"
#include "wifi.h"

// Ensure LV_FS_MAX_PATH_LENGTH is defined
#ifndef LV_FS_MAX_PATH_LENGTH
#define LV_FS_MAX_PATH_LENGTH 256
#endif

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

        lv_obj_t* album_cover_container;
        lv_obj_t* album_cover;
        lv_obj_t* vinyl_ring;
        lv_obj_t* vinyl_center;
        lv_obj_t* album_name;
        lv_obj_t* album_artist;

        lv_obj_t* play_btn;
        lv_obj_t* playback_group;
        lv_obj_t* playback_progress;
        lv_span_t* playback_current_time;
        lv_span_t* playback_total_time;

        lv_obj_t* playlist;
        lv_obj_t* frosted_bg;
        lv_obj_t* backward_10s_btn;
        lv_obj_t* forward_10s_btn;
        lv_obj_t* wifi_status_label;
    } ui;

    struct {
        struct {
            const lv_font_t* normal;
        } size_16;
        struct {
            const lv_font_t* bold;
        } size_22;
        struct {
            const lv_font_t* normal;
        } size_24;
        struct {
            const lv_font_t* normal;
        } size_28;
        struct {
            const lv_font_t* bold;
        } size_60;
    } fonts;

    struct {
        lv_style_t button_default;
        lv_style_t button_pressed;
        lv_style_t circular_cover;
        lv_style_t vinyl_ring;
        lv_style_t vinyl_center;
        lv_style_t gradient_progress;
        lv_style_t frosted_glass;
        lv_style_t modern_card;             // Modern card style
        lv_style_transition_dsc_t button_transition_dsc;
        lv_style_transition_dsc_t transition_dsc;
        lv_style_transition_dsc_t cover_rotation;     // Cover rotation animation
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
        const char* background;  // Background image
    } images;

    album_info_t* albums;
    uint8_t album_count;
};

struct ctx_s {
    bool resource_healthy_check;

    album_info_t* current_album;
    lv_obj_t* current_album_related_obj;

    uint16_t volume;

    play_status_t play_status_prev;
    play_status_t play_status;
    uint64_t current_time;

    struct {
        lv_timer_t* volume_bar_countdown;
        lv_timer_t* playback_progress_update;
        lv_timer_t* refresh_date_time;       // Date time update timer
        lv_timer_t* cover_rotation;          // Cover rotation timer
    } timers;

    struct {
        lv_anim_t cover_rotation_anim;       // Cover rotation animation
        bool is_rotating;                    // Whether currently rotating
        int16_t rotation_angle;              // Current rotation angle
    } animations;

    audioctl_s* audioctl;
};

struct conf_s {
#if WIFI_ENABLED
    wifi_conf_t wifi;
#endif
};

void app_create(void);
void splash_screen_create(void);  // Splash screen creation function

// Unified playlist manager functions (optimized version)
void playlist_manager_create(lv_obj_t* parent);
void playlist_manager_refresh(void);
void playlist_manager_close(void);
bool playlist_manager_is_open(void);

// app control API for external modules
void app_set_play_status(play_status_t status);
void app_switch_to_album(int index);

// WiFi optimization functions (v1.1.2 new)
int wifi_manager_optimized_init(void);
int wifi_connect_optimized(const char* ssid, const char* password);
void wifi_start_connection_monitor(void);
void wifi_set_auto_reconnect(bool enabled);
void wifi_create_settings_ui(lv_obj_t* parent);
void wifi_manager_optimized_cleanup(void);

// Internal function declarations
void app_switch_to_album(int index);

#endif // LVGL_APP_H
