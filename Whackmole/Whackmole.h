#ifndef WHACKMOLE_H
#define WHACKMOLE_H
#include <lvgl/lvgl.h>
#define RES_ROOT CONFIG_LVX_WHACKMOLE_DATA_ROOT "/pic"
#define ICONS_ROOT RES_ROOT "/icons"
#define FONTS_ROOT RES_ROOT "/fonts"
void init_whack_a_mole_game(lv_obj_t* parent);
void whackmole_app_create(void);
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
        } size_14;
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
        const char* hammer;
        const char* mole;
        const char* grassland;
    } images;
};
struct ctx_s {
    bool resource_healthy_check;
};
#endif // WHACKMOLE_H
