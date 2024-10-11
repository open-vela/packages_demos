//
// Created by BenignX on 2024/3/21.
//

/*
 * UI:
 *
 * TIME GROUP:
 *      TIME: 00:00:00
 *      DATE: 2024/03/21
 *
 * PLAYER GROUP:
 *      ALBUM GROUP:
 *          ALBUM PICTURE
 *          ALBUM INFO:
 *              ALBUM NAME
 *              ALBUM ARTIST
 *      PROGRESS GROUP:
 *          CURRENT TIME: 00:00/00:00
 *          PLAYBACK PROGRESS BAR
 *      CONTROL GROUP:
 *          PLAYLIST
 *          PREVIOUS
 *          PLAY/PAUSE
 *          NEXT
 *          AUDIO
 *
 * TOP Layer:
 *      VOLUME BAR
 *      PLAYLIST GROUP:
 *          TITLE
 *          LIST:
 *              ICON
 *              ALBUM NAME
 *              ALBUM ARTIST
 */

/*********************
 *      INCLUDES
 *********************/

#include "music_player.h"
#include <netutils/cJSON.h>
#include <time.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/* Init functions */
static void read_configs(void);
static void init_resource(void);
static void reload_music_config(void);
static void app_create_main_page(void);
static void app_create_top_layer(void);

/* Timer starting functions */
static void app_start_updating_date_time(void);

/* Album operations */
static int32_t app_get_album_index(album_info_t* album);
static void app_switch_to_album(int index);

/* Album operations */
static void app_set_play_status(play_status_t status);
static void app_set_playback_time(uint32_t current_time);
static void app_set_volume(uint16_t volume);

/* UI refresh functions */
static void app_refresh_album_info(void);
static void app_refresh_date_time(void);
static void app_refresh_play_status(void);
static void app_refresh_playback_progress(void);
static void app_refresh_playlist(void);
static void app_refresh_volume_bar(void);
static void app_refresh_volume_countdown_timer(void);

/* Event handler functions */
static void app_audio_event_handler(lv_event_t* e);
static void app_play_status_event_handler(lv_event_t* e);
static void app_playlist_btn_event_handler(lv_event_t* e);
static void app_playlist_event_handler(lv_event_t* e);
static void app_switch_album_event_handler(lv_event_t* e);
static void app_volume_bar_event_handler(lv_event_t* e);
static void app_playback_progress_bar_event_handler(lv_event_t* e);

/* Timer callback functions */
static void app_refresh_date_time_timer_cb(lv_timer_t* timer);
static void app_playback_progress_update_timer_cb(lv_timer_t* timer);
static void app_volume_bar_countdown_timer_cb(lv_timer_t* timer);

/**********************
 *  STATIC VARIABLES
 **********************/

// clang-format off
struct resource_s   R;  /**< Resources */
struct ctx_s        C;  /**< Context */
struct conf_s       CF; /**< Configuration */
// clang-format on

/* Week days mapping */
const char* WEEK_DAYS[] = { "Sun.", "Mon.", "Tues.", "Wed.", "Thur.", "Fri.", "Sat." };

/* Transition properties for the objects */
const lv_style_prop_t transition_props[] = {
    LV_STYLE_OPA,
    LV_STYLE_BG_OPA,
    LV_STYLE_Y,
    LV_STYLE_HEIGHT,
    LV_STYLE_PROP_FLAG_NONE
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void app_create(void)
{
    // Init resource and context structure
    lv_memzero(&R, sizeof(R));
    lv_memzero(&C, sizeof(C));
    lv_memzero(&CF, sizeof(CF));

    read_configs();

#if WIFI_ENABLED
    CF.wifi.conn_delay = 2000000;
    wifi_connect(&CF.wifi);
#endif

    app_create_main_page();
    app_set_play_status(PLAY_STATUS_STOP);
    app_switch_to_album(0);
    app_set_volume(30);

    app_refresh_album_info();
    app_refresh_playlist();
    app_refresh_volume_bar();

    app_start_updating_date_time();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int32_t app_get_album_index(album_info_t* album)
{
    for (int i = 0; i < R.album_count; i++) {
        if (album == &R.albums[i]) {
            return i;
        }
    }
    return -1;
}

static void app_set_volume(uint16_t volume)
{
    C.volume = volume;
    audio_ctl_set_volume(C.audioctl, C.volume);
}

static void app_set_play_status(play_status_t status)
{
    C.play_status_prev = C.play_status;
    C.play_status = status;
    app_refresh_play_status();
}

static void app_switch_to_album(int index)
{
    if (R.album_count == 0 || index < 0 || index >= R.album_count || C.current_album == &R.albums[index])
        return;

    C.current_album = &R.albums[index];
    app_refresh_album_info();
    app_refresh_playlist();
    app_set_playback_time(0);

    if (C.play_status == PLAY_STATUS_STOP) {
        return;
    }

    app_set_play_status(PLAY_STATUS_STOP);
    app_set_play_status(PLAY_STATUS_PLAY);
}

static void app_set_playback_time(uint32_t current_time)
{
    C.current_time = current_time;

    audio_ctl_seek(C.audioctl, C.current_time / 1000);
    app_refresh_playback_progress();
}

static void app_refresh_date_time(void)
{
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);

    char date_str[20];
    strftime(date_str, sizeof(date_str), "%Y.%m.%d", timeinfo);
    lv_snprintf(date_str + 10, sizeof(date_str), " %s", WEEK_DAYS[timeinfo->tm_wday]);
    lv_label_set_text(R.ui.date, date_str);

    char time_str[9];
    strftime(time_str, sizeof(time_str), "%H:%M", timeinfo);
    lv_label_set_text(R.ui.time, time_str);
}

static void app_refresh_volume_bar(void)
{
    int32_t volume_bar_indic_height = C.volume;

    lv_obj_set_height(R.ui.volume_bar_indic, volume_bar_indic_height);

    lv_obj_refr_size(R.ui.volume_bar_indic);
    lv_obj_update_layout(R.ui.volume_bar_indic);

    if (C.volume > 0) {
        lv_image_set_src(R.ui.audio, R.images.audio);
    } else {
        lv_image_set_src(R.ui.audio, R.images.mute);
    }
}

static void app_refresh_album_info(void)
{
    if (C.current_album) {
        if (access(C.current_album->cover, F_OK) == 0)
            lv_image_set_src(R.ui.album_cover, C.current_album->cover);
        else
            lv_image_set_src(R.ui.album_cover, R.images.nocover);
        lv_label_set_text(R.ui.album_name, C.current_album->name);
        lv_label_set_text(R.ui.album_artist, C.current_album->artist);
    }
}

static void app_refresh_play_status(void)
{
    if (C.timers.playback_progress_update == NULL) {
        C.timers.playback_progress_update = lv_timer_create(app_playback_progress_update_timer_cb, 1000, NULL);
    }

    switch (C.play_status) {
    case PLAY_STATUS_STOP:
        lv_image_set_src(R.ui.play_btn, R.images.play);
        lv_timer_pause(C.timers.playback_progress_update);
        if (C.audioctl) {
            audio_ctl_stop(C.audioctl);
            audio_ctl_uninit_nxaudio(C.audioctl);
            C.audioctl = NULL;
        }
        break;
    case PLAY_STATUS_PLAY:
        lv_image_set_src(R.ui.play_btn, R.images.pause);
        lv_timer_resume(C.timers.playback_progress_update);
        if (C.play_status_prev == PLAY_STATUS_PAUSE)
            audio_ctl_resume(C.audioctl);
        else if (C.play_status_prev == PLAY_STATUS_STOP) {
            C.audioctl = audio_ctl_init_nxaudio(C.current_album->path);
            audio_ctl_start(C.audioctl);
        }
        break;
    case PLAY_STATUS_PAUSE:
        lv_image_set_src(R.ui.play_btn, R.images.play);
        lv_timer_pause(C.timers.playback_progress_update);
        audio_ctl_pause(C.audioctl);
        break;
    default:
        break;
    }
}

static void app_refresh_playback_progress(void)
{
    uint64_t total_time = C.current_album->total_time;

    if (C.current_time > total_time) {
        app_set_play_status(PLAY_STATUS_STOP);
        C.current_time = 0;
        return;
    }

    lv_bar_set_range(R.ui.playback_progress, 0, (int32_t)total_time);
    lv_bar_set_value(R.ui.playback_progress, (int32_t)C.current_time, LV_ANIM_ON);

    char buff[256];

    uint32_t current_time_min = C.current_time / 60000;
    uint32_t current_time_sec = (C.current_time % 60000) / 1000;
    uint32_t total_time_min = total_time / 60000;
    uint32_t total_time_sec = (total_time % 60000) / 1000;

    lv_snprintf(buff, sizeof(buff), "%02d:%02d", current_time_min, current_time_sec);
    lv_span_set_text(R.ui.playback_current_time, buff);
    lv_snprintf(buff, sizeof(buff), "%02d:%02d", total_time_min, total_time_sec);
    lv_span_set_text(R.ui.playback_total_time, buff);
}

static void app_refresh_playlist(void)
{
    lv_obj_clean(R.ui.playlist);
    for (int i = 0; i < R.album_count; i++) {
        // LIST ITEM
        lv_obj_t* list_item = lv_obj_create(R.ui.playlist);
        lv_obj_remove_style_all(list_item);
        lv_obj_set_size(list_item, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_margin_all(list_item, 20, LV_PART_MAIN);
        lv_obj_set_flex_flow(list_item, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(list_item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // ICON
        if (C.current_album == &R.albums[i]) {
            lv_obj_t* icon = lv_image_create(list_item);
            lv_image_set_src(icon, R.images.music);
            lv_obj_set_style_bg_opa(icon, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_radius(icon, 16, LV_PART_MAIN);
            lv_obj_set_style_margin_right(icon, 12, LV_PART_MAIN);
            lv_obj_align(icon, LV_ALIGN_TOP_LEFT, 10, 10);

            C.current_album_related_obj = list_item;
        }

        // ALBUM NAME
        lv_obj_t* album_info_spangroup = lv_spangroup_create(list_item);
        lv_obj_set_size(album_info_spangroup, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_add_flag(album_info_spangroup, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_spangroup_set_overflow(album_info_spangroup, LV_SPAN_OVERFLOW_ELLIPSIS);

        lv_span_t* album_name = lv_spangroup_new_span(album_info_spangroup);
        lv_span_set_text(album_name, R.albums[i].name);
        lv_obj_set_style_text_font(album_info_spangroup, R.fonts.size_24.normal, LV_PART_MAIN);
        lv_obj_set_style_text_color(album_info_spangroup, lv_color_white(), LV_PART_MAIN);

        // ALBUM ARTIST DOT
        lv_span_t* album_artist_dot = lv_spangroup_new_span(album_info_spangroup);
        lv_span_set_text(album_artist_dot, "·");
        lv_style_set_text_color(&album_artist_dot->style, lv_color_hex(0x878787));

        // ALBUM ARTIST
        lv_span_t* album_artist = lv_spangroup_new_span(album_info_spangroup);
        lv_span_set_text(album_artist, R.albums[i].artist);
        lv_style_set_text_color(&album_artist->style, lv_color_hex(0x878787));

        // EVENTS
        lv_obj_add_event_cb(list_item, app_playlist_btn_event_handler, LV_EVENT_CLICKED, &R.albums[i]);
    }

    if (C.current_album_related_obj)
        lv_obj_scroll_to_view(C.current_album_related_obj, LV_ANIM_ON);
}

static void app_volume_bar_countdown_timer_cb(lv_timer_t* timer)
{
    LV_UNUSED(timer);
    lv_obj_set_state(R.ui.volume_bar, LV_STATE_DEFAULT, true);
    lv_obj_set_state(R.ui.volume_bar, LV_STATE_USER_1, false);
}

static void app_playback_progress_update_timer_cb(lv_timer_t* timer)
{
    LV_UNUSED(timer);

    C.current_time = audio_ctl_get_position(C.audioctl) * 1000;
    app_refresh_playback_progress();
}

static void app_refresh_date_time_timer_cb(lv_timer_t* timer)
{
    LV_UNUSED(timer);

    app_refresh_date_time();
}

static void app_refresh_volume_countdown_timer(void)
{
    if (C.timers.volume_bar_countdown) {
        lv_timer_set_repeat_count(C.timers.volume_bar_countdown, 1);
        lv_timer_reset(C.timers.volume_bar_countdown);
        lv_timer_resume(C.timers.volume_bar_countdown);
    } else {
        C.timers.volume_bar_countdown = lv_timer_create(app_volume_bar_countdown_timer_cb, 3000, NULL);
        lv_timer_set_auto_delete(C.timers.volume_bar_countdown, false);
    }
}

static void app_playlist_event_handler(lv_event_t* e)
{
    LV_UNUSED(e);

    lv_obj_t* top_layer = lv_layer_top();

    int32_t screen_height = lv_obj_get_height(lv_screen_active());

    if (lv_obj_has_state(R.ui.playlist_base, LV_STATE_USER_1)) {
        lv_obj_set_style_y(R.ui.playlist_base, screen_height, LV_STATE_DEFAULT);
        lv_obj_set_state(R.ui.playlist_base, LV_STATE_DEFAULT, true);
        lv_obj_set_state(R.ui.playlist_base, LV_STATE_USER_1, false);
        lv_obj_set_state(top_layer, LV_STATE_DEFAULT, true);
        lv_obj_set_state(top_layer, LV_STATE_USER_1, false);
    } else {
        lv_obj_set_state(R.ui.playlist_base, LV_STATE_DEFAULT, false);
        lv_obj_set_state(R.ui.playlist_base, LV_STATE_USER_1, true);
        lv_obj_set_state(top_layer, LV_STATE_DEFAULT, false);
        lv_obj_set_state(top_layer, LV_STATE_USER_1, true);
    }
}

static void app_volume_bar_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (!(code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING || code == LV_EVENT_PRESS_LOST)) {
        return;
    }

    lv_point_t point;
    lv_indev_t* indev = lv_indev_active();
    lv_indev_get_vect(indev, &point);

    int32_t volume_bar_height = lv_obj_get_height(R.ui.volume_bar);
    int32_t volume_bar_indic_height = lv_obj_get_height(R.ui.volume_bar_indic);

    if (volume_bar_indic_height < 0) {
        volume_bar_indic_height = 0;
    } else if (volume_bar_indic_height > volume_bar_height) {
        volume_bar_indic_height = volume_bar_height;
    }

    int32_t volume = volume_bar_indic_height - point.y;
    if (volume < 0)
        volume = 0;

    app_set_volume(volume);

    app_refresh_volume_bar();
    app_refresh_volume_countdown_timer();
}

static void app_audio_event_handler(lv_event_t* e)
{
    LV_UNUSED(e);

    if (lv_obj_has_state(R.ui.volume_bar, LV_STATE_USER_1)) {
        lv_obj_set_state(R.ui.volume_bar, LV_STATE_DEFAULT, true);
        lv_obj_set_state(R.ui.volume_bar, LV_STATE_USER_1, false);
    } else {
        lv_obj_set_state(R.ui.volume_bar, LV_STATE_DEFAULT, false);
        lv_obj_set_state(R.ui.volume_bar, LV_STATE_USER_1, true);
        app_refresh_volume_countdown_timer();
    }
}

static void app_playlist_btn_event_handler(lv_event_t* e)
{
    album_info_t* album_info = (album_info_t*)lv_event_get_user_data(e);

    int32_t index = app_get_album_index(album_info);
    if (index >= 0) {
        app_switch_to_album(index);
        app_playlist_event_handler(NULL);
    }
}

static void app_switch_album_event_handler(lv_event_t* e)
{
    switch_album_mode_t direction = (switch_album_mode_t)(lv_uintptr_t)lv_event_get_user_data(e);

    int32_t album_index = app_get_album_index(C.current_album);
    if (album_index < 0) {
        return;
    }

    switch (direction) {
    case SWITCH_ALBUM_MODE_PREV:
        album_index--;
        break;
    case SWITCH_ALBUM_MODE_NEXT:
        album_index++;
        break;
    default:
        break;
    }

    album_index = (album_index + R.album_count) % R.album_count;

    app_switch_to_album(album_index);
}

static void app_play_status_event_handler(lv_event_t* e)
{
    LV_UNUSED(e);

    switch (C.play_status) {
    case PLAY_STATUS_STOP:
        app_set_play_status(PLAY_STATUS_PLAY);
        break;
    case PLAY_STATUS_PLAY:
        app_set_play_status(PLAY_STATUS_PAUSE);
        break;
    case PLAY_STATUS_PAUSE:
        app_set_play_status(PLAY_STATUS_PLAY);
        break;
    default:
        break;
    }
}

static void app_playback_progress_bar_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    switch (code) {
    case LV_EVENT_LONG_PRESSED:
        lv_obj_set_height(R.ui.playback_progress, 8);
        lv_obj_set_style_margin_ver(R.ui.playback_progress, 0, LV_PART_MAIN);
        lv_obj_set_state(R.ui.playback_progress, LV_STATE_CHECKED, true);
        break;
    case LV_EVENT_PRESS_LOST:
    case LV_EVENT_RELEASED:
        lv_obj_set_height(R.ui.playback_progress, 4);
        lv_obj_set_style_margin_ver(R.ui.playback_progress, 2, LV_PART_MAIN);
        lv_obj_set_state(R.ui.playback_progress, LV_STATE_CHECKED, false);
        break;
    case LV_EVENT_PRESSING: {
        if (lv_obj_has_state(R.ui.playback_progress, LV_STATE_CHECKED)) {
            lv_point_t point;
            lv_point_t vec;
            lv_indev_t* indev = lv_indev_active();
            lv_indev_get_point(indev, &point);
            lv_indev_get_vect(indev, &vec);

            if (point.y <= 0) {
                return;
            }

            int32_t screen_height = lv_obj_get_height(lv_screen_active());
            int32_t screen_width = lv_obj_get_width(lv_screen_active());
            int32_t level_height = screen_height / 10;
            int32_t level = point.y / level_height + 1;

            uint64_t total_time = C.current_album ? C.current_album->total_time : 0;

            C.current_time += vec.x * level * (int32_t)total_time / screen_width / 10;
            app_set_playback_time(C.current_time);
        }
        break;
    }
    default:
        break;
    }
}

static void init_resource(void)
{
    // Fonts
    R.fonts.size_16.normal = lv_freetype_font_create(FONTS_ROOT "/MiSans-Normal.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 16, LV_FREETYPE_FONT_STYLE_NORMAL);
    R.fonts.size_22.bold = lv_freetype_font_create(FONTS_ROOT "/MiSans-Semibold.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 22, LV_FREETYPE_FONT_STYLE_NORMAL);
    R.fonts.size_24.normal = lv_freetype_font_create(FONTS_ROOT "/MiSans-Normal.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 24, LV_FREETYPE_FONT_STYLE_NORMAL);
    R.fonts.size_28.normal = lv_freetype_font_create(FONTS_ROOT "/MiSans-Semibold.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 38, LV_FREETYPE_FONT_STYLE_NORMAL);
    R.fonts.size_60.bold = lv_freetype_font_create(FONTS_ROOT "/MiSans-Semibold.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 60, LV_FREETYPE_FONT_STYLE_NORMAL);

    // Styles
    lv_style_init(&R.styles.button_default);
    lv_style_init(&R.styles.button_pressed);
    lv_style_set_opa(&R.styles.button_default, LV_OPA_COVER);
    lv_style_set_opa(&R.styles.button_pressed, LV_OPA_70);

    // transition animation
    lv_style_transition_dsc_init(&R.styles.transition_dsc, transition_props, &lv_anim_path_ease_in_out, 300, 0, NULL);
    lv_style_transition_dsc_init(&R.styles.button_transition_dsc, transition_props, &lv_anim_path_ease_in_out, 80, 0, NULL);
    lv_style_set_transition(&R.styles.button_default, &R.styles.button_transition_dsc);
    lv_style_set_transition(&R.styles.button_pressed, &R.styles.button_transition_dsc);

    // images
    R.images.playlist = ICONS_ROOT "/playlist.png";
    R.images.previous = ICONS_ROOT "/previous.png";
    R.images.play = ICONS_ROOT "/play.png";
    R.images.pause = ICONS_ROOT "/pause.png";
    R.images.next = ICONS_ROOT "/next.png";
    R.images.audio = ICONS_ROOT "/audio.png";
    R.images.mute = ICONS_ROOT "/mute.png";
    R.images.music = ICONS_ROOT "/music.png";
    R.images.nocover = ICONS_ROOT "/nocover.png";

    // albums
    reload_music_config();
}

static void app_create_top_layer(void)
{
    lv_obj_t* top_layer = lv_layer_top();
    lv_obj_set_scroll_dir(top_layer, LV_DIR_NONE);
    lv_obj_set_style_bg_color(top_layer, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(top_layer, LV_OPA_COVER, LV_STATE_USER_1);
    lv_obj_set_style_bg_opa(top_layer, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_transition(top_layer, &R.styles.transition_dsc, LV_STATE_DEFAULT);
    lv_obj_set_style_transition(top_layer, &R.styles.transition_dsc, LV_STATE_USER_1);

    // VOLUME BAR
    R.ui.volume_bar = lv_obj_create(top_layer);
    lv_obj_remove_style_all(R.ui.volume_bar);
    lv_obj_set_size(R.ui.volume_bar, 60, 180);
    lv_obj_set_style_bg_color(R.ui.volume_bar, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(R.ui.volume_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_opa(R.ui.volume_bar, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_opa(R.ui.volume_bar, LV_OPA_COVER, LV_STATE_USER_1);
    lv_obj_set_style_border_width(R.ui.volume_bar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(R.ui.volume_bar, 16, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(R.ui.volume_bar, true, LV_PART_MAIN);
    lv_obj_align(R.ui.volume_bar, LV_ALIGN_BOTTOM_RIGHT, -45, -95);
    lv_obj_set_style_transition(R.ui.volume_bar, &R.styles.transition_dsc, LV_STATE_DEFAULT);

    R.ui.volume_bar_indic = lv_obj_create(R.ui.volume_bar);
    lv_obj_remove_style_all(R.ui.volume_bar_indic);
    lv_obj_set_style_bg_color(R.ui.volume_bar_indic, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(R.ui.volume_bar_indic, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_size(R.ui.volume_bar_indic, LV_PCT(100), 40);
    lv_obj_align(R.ui.volume_bar_indic, LV_ALIGN_BOTTOM_MID, 0, 0);

    // PLAYLIST GROUP
    R.ui.playlist_base = lv_obj_create(top_layer);
    lv_obj_remove_style_all(R.ui.playlist_base);
    lv_obj_set_size(R.ui.playlist_base, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_y(R.ui.playlist_base, 480, LV_STATE_DEFAULT);
    lv_obj_set_style_y(R.ui.playlist_base, 0, LV_STATE_USER_1);
    lv_obj_set_flex_flow(R.ui.playlist_base, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_top(R.ui.playlist_base, 60, LV_PART_MAIN);
    lv_obj_set_style_pad_left(R.ui.playlist_base, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_right(R.ui.playlist_base, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(R.ui.playlist_base, 10, LV_PART_MAIN);
    lv_obj_set_flex_align(R.ui.playlist_base, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* playlist_group = lv_obj_create(R.ui.playlist_base);
    lv_obj_remove_style_all(playlist_group);
    lv_obj_set_style_bg_color(playlist_group, lv_color_hex(0x1E1E20), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(playlist_group, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_size(playlist_group, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_radius(playlist_group, 16, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(playlist_group, true, LV_PART_MAIN);
    lv_obj_set_flex_flow(playlist_group, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(playlist_group, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // TITLE
    lv_obj_t* title = lv_label_create(playlist_group);
    lv_label_set_text(title, "PLAYLIST");
    lv_obj_set_style_text_font(title, R.fonts.size_28.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_margin_all(title, 15, LV_PART_MAIN);

    // LIST
    lv_obj_t* list = lv_obj_create(playlist_group);
    R.ui.playlist = list;
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x1E1E20), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(list, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // EVENTS
    lv_obj_add_event_cb(R.ui.playlist_base, app_playlist_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_transition(R.ui.playlist_base, &R.styles.transition_dsc, LV_STATE_DEFAULT);
    lv_obj_set_style_transition(R.ui.playlist_base, &R.styles.transition_dsc, LV_STATE_USER_1);

    lv_obj_add_flag(R.ui.volume_bar_indic, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(R.ui.volume_bar, app_volume_bar_event_handler, LV_EVENT_ALL, NULL);
}

static void app_create_main_page(void)
{
    init_resource();

    lv_obj_t* root = lv_screen_active();
    lv_obj_set_style_bg_color(root, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(root, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(root, 20, LV_PART_MAIN);

    // TIME GROUP
    lv_obj_t* time_group = lv_obj_create(root);
    lv_obj_set_size(time_group, LV_PCT(100), LV_PCT(50));
    lv_obj_set_style_bg_color(time_group, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(time_group, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(time_group, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(time_group, 1);
    lv_obj_set_flex_flow(time_group, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(time_group, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // TIME
    lv_obj_t* time = lv_label_create(time_group);
    R.ui.time = time;
    lv_label_set_text(time, "13:14");
    lv_obj_set_style_text_font(time, R.fonts.size_60.bold, LV_PART_MAIN);
    lv_obj_set_style_text_color(time, lv_color_white(), LV_PART_MAIN);

    // DATE
    lv_obj_t* date = lv_label_create(time_group);
    R.ui.date = date;
    lv_label_set_text(date, "2024.03.22 Fri.");
    lv_obj_set_style_text_font(date, R.fonts.size_24.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(date, lv_color_hex(0x878787), LV_PART_MAIN);

    // PLAYER GROUP
    lv_obj_t* player_group = lv_obj_create(root);
    R.ui.player_group = player_group;
    lv_obj_set_size(player_group, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(player_group, lv_color_hex(0x1E1E20), LV_PART_MAIN);
    lv_obj_set_style_border_width(player_group, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(player_group, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_all(player_group, 20, LV_PART_MAIN);
    lv_obj_set_flex_flow(player_group, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(player_group, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // ALBUM GROUP
    lv_obj_t* album_group = lv_obj_create(player_group);
    lv_obj_remove_style_all(album_group);
    lv_obj_set_size(album_group, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_flex_flow(album_group, LV_FLEX_FLOW_ROW, LV_PART_MAIN);
    lv_obj_set_flex_align(album_group, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // ALBUM PICTURE
    lv_obj_t* album_cover = lv_obj_create(album_group);
    lv_obj_t* album_cover_img = lv_image_create(album_cover);
    R.ui.album_cover = album_cover_img;
    lv_obj_remove_style_all(album_cover);
    lv_obj_set_size(album_cover, 80, 80);
    lv_obj_set_size(album_cover_img, 80, 80);
    lv_obj_set_style_radius(album_cover, 12, LV_PART_MAIN);
    lv_obj_set_style_radius(album_cover_img, 12, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(album_cover, true, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(album_cover_img, true, LV_PART_MAIN);
    lv_image_set_align(album_cover_img, LV_IMAGE_ALIGN_STRETCH);
    lv_image_set_src(album_cover_img, R.images.nocover);

    // ALBUM INFO
    lv_obj_t* album_info = lv_obj_create(album_group);
    lv_obj_remove_style_all(album_info);
    lv_obj_remove_flag(album_info, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(album_info, LV_SIZE_CONTENT, 80);
    lv_obj_set_style_pad_left(album_info, 20, LV_PART_MAIN);
    lv_obj_set_flex_grow(album_info, 1);
    lv_obj_set_style_flex_flow(album_info, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN);
    lv_obj_set_flex_align(album_info, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // ALBUM NAME
    lv_obj_t* album_name = lv_label_create(album_info);
    R.ui.album_name = album_name;
    lv_label_set_long_mode(album_name, LV_LABEL_LONG_SCROLL);
    lv_obj_set_width(album_name, LV_PCT(100));
    lv_obj_set_style_text_align(album_name, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(album_name, R.fonts.size_22.bold, LV_PART_MAIN);
    lv_obj_set_style_text_color(album_name, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(album_name, "No Album");

    // ALBUM ARTIST
    lv_obj_t* album_artist = lv_label_create(album_info);
    R.ui.album_artist = album_artist;
    lv_label_set_long_mode(album_name, LV_LABEL_LONG_SCROLL);
    lv_obj_set_width(album_artist, LV_PCT(100));
    lv_obj_set_style_text_align(album_artist, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(album_artist, R.fonts.size_22.bold, LV_PART_MAIN);
    lv_obj_set_style_text_color(album_artist, lv_color_hex3(0x666), LV_PART_MAIN);
    lv_label_set_text(album_artist, "No Artist");

    // PROGRESS BAR GROUP
    lv_obj_t* progress_group = lv_obj_create(player_group);
    R.ui.playback_group = progress_group;
    lv_obj_remove_style_all(progress_group);
    lv_obj_set_size(progress_group, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_margin_top(progress_group, -16, LV_PART_MAIN);
    lv_obj_set_style_flex_flow(progress_group, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN);
    lv_obj_set_flex_align(progress_group, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* progress_time_spangroup = lv_spangroup_create(progress_group);
    lv_span_t* current_time_span = lv_spangroup_new_span(progress_time_spangroup);
    lv_span_t* slash_span = lv_spangroup_new_span(progress_time_spangroup);
    lv_span_t* total_time_span = lv_spangroup_new_span(progress_time_spangroup);
    R.ui.playback_current_time = current_time_span;
    R.ui.playback_total_time = total_time_span;
    lv_span_set_text(current_time_span, "00:00");
    lv_span_set_text(slash_span, "/");
    lv_span_set_text(total_time_span, "00:00");
    lv_obj_set_style_text_font(progress_time_spangroup, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(progress_time_spangroup, lv_color_white(), LV_PART_MAIN);
    lv_style_set_text_color(&slash_span->style, lv_color_hex(0x666666));
    lv_style_set_text_color(&total_time_span->style, lv_color_hex(0x666666));

    lv_obj_t* progress_bar = lv_bar_create(progress_group);
    R.ui.playback_progress = progress_bar;
    lv_obj_set_size(progress_bar, LV_PCT(100), 4);
    lv_obj_add_flag(progress_bar, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_margin_ver(progress_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress_bar, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(progress_bar, LV_OPA_COVER, LV_PART_MAIN);

    // CONTROL GROUP
    lv_obj_t* control_group = lv_obj_create(player_group);
    lv_obj_remove_style_all(control_group);
    lv_obj_set_size(control_group, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_flex_flow(control_group, LV_FLEX_FLOW_ROW, LV_PART_MAIN);
    lv_obj_set_flex_align(control_group, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // PLAYLIST
    lv_obj_t* playlist = lv_button_create(control_group);
    lv_obj_t* playlist_image = lv_image_create(playlist);
    lv_obj_remove_style_all(playlist);
    lv_image_set_src(playlist_image, R.images.playlist);
    lv_obj_set_size(playlist_image, 66, 66);
    lv_obj_add_style(playlist, &R.styles.button_default, LV_STATE_DEFAULT);
    lv_obj_add_style(playlist, &R.styles.button_pressed, LV_STATE_PRESSED);

    // PREVIOUS
    lv_obj_t* previous = lv_button_create(control_group);
    lv_obj_t* previous_image = lv_image_create(previous);
    lv_obj_remove_style_all(previous);
    lv_image_set_src(previous_image, R.images.previous);
    lv_obj_set_size(previous_image, 66, 66);
    lv_obj_add_style(previous, &R.styles.button_default, LV_STATE_DEFAULT);
    lv_obj_add_style(previous, &R.styles.button_pressed, LV_STATE_PRESSED);

    // PLAY/PAUSE
    lv_obj_t* play_pause = lv_button_create(control_group);
    lv_obj_t* play_pause_image = lv_image_create(play_pause);
    R.ui.play_btn = play_pause_image;
    lv_obj_remove_style_all(play_pause);
    lv_image_set_src(play_pause_image, R.images.play);
    lv_obj_set_size(play_pause_image, 66, 66);
    lv_obj_add_style(play_pause, &R.styles.button_default, LV_STATE_DEFAULT);
    lv_obj_add_style(play_pause, &R.styles.button_pressed, LV_STATE_PRESSED);

    // NEXT
    lv_obj_t* next = lv_button_create(control_group);
    lv_obj_t* next_image = lv_image_create(next);
    lv_obj_remove_style_all(next);
    lv_image_set_src(next_image, R.images.next);
    lv_obj_set_size(next_image, 66, 66);
    lv_obj_add_style(next, &R.styles.button_default, LV_STATE_DEFAULT);
    lv_obj_add_style(next, &R.styles.button_pressed, LV_STATE_PRESSED);

    // AUDIO
    lv_obj_t* audio = lv_button_create(control_group);
    lv_obj_t* audio_image = lv_image_create(audio);
    R.ui.audio = audio_image;
    lv_obj_remove_style_all(audio);
    lv_obj_set_size(audio_image, 66, 66);
    lv_obj_add_style(audio, &R.styles.button_default, LV_STATE_DEFAULT);
    lv_obj_add_style(audio, &R.styles.button_pressed, LV_STATE_PRESSED);

    app_create_top_layer();

    lv_obj_add_event_cb(playlist, app_playlist_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(audio, app_audio_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(play_pause, app_play_status_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(previous, app_switch_album_event_handler, LV_EVENT_CLICKED, (lv_uintptr_t*)SWITCH_ALBUM_MODE_PREV);
    lv_obj_add_event_cb(next, app_switch_album_event_handler, LV_EVENT_CLICKED, (lv_uintptr_t*)SWITCH_ALBUM_MODE_NEXT);
    lv_obj_add_event_cb(progress_group, app_playback_progress_bar_event_handler, LV_EVENT_ALL, NULL);
}

static void app_start_updating_date_time(void)
{
    lv_timer_t* timer = lv_timer_create(app_refresh_date_time_timer_cb, 1000, NULL);
    lv_timer_ready(timer);
}

static void read_configs(void)
{
    uint32_t file_size;
    lv_fs_file_t file;
    lv_fs_open(&file, RES_ROOT "/config.json", LV_FS_MODE_RD);

    lv_fs_seek(&file, 0, LV_FS_SEEK_END);
    lv_fs_tell(&file, &file_size);
    lv_fs_seek(&file, 0, LV_FS_SEEK_SET);

    char* buff = lv_malloc(file_size);
    lv_fs_read(&file, buff, file_size, NULL);

    const char* json_string = buff;

    cJSON* json = cJSON_Parse(json_string);
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            LV_LOG_ERROR("parse error: %p", error_ptr);
        }
        lv_free(buff);
        return;
    }

#if WIFI_ENABLED
    cJSON* wifi_object = cJSON_GetObjectItem(json, "wifi");

    const char* ssid = cJSON_GetStringValue(cJSON_GetObjectItem(wifi_object, "ssid"));
    const char* pswd = cJSON_GetStringValue(cJSON_GetObjectItem(wifi_object, "pswd"));
    const int version = cJSON_GetNumberValue(cJSON_GetObjectItem(wifi_object, "wpa_ver"));

    lv_strcpy(CF.wifi.ssid, ssid);
    lv_strcpy(CF.wifi.pswd, pswd);
    CF.wifi.ver_flag = version;
#endif

    cJSON_Delete(json);

    lv_free(buff);
}

static void reload_music_config(void)
{

    /* Clear previous music config */

    for (int i = 0; i < R.album_count; i++) {
        lv_free((void*)R.albums[i].name);
        lv_free((void*)R.albums[i].artist);
    }

    lv_free(R.albums);
    R.album_count = 0;

    /* Load music config */
    uint32_t file_size;
    lv_fs_file_t file;
    lv_fs_open(&file, MUSICS_ROOT "/manifest.json", LV_FS_MODE_RD);

    lv_fs_seek(&file, 0, LV_FS_SEEK_END);
    lv_fs_tell(&file, &file_size);
    lv_fs_seek(&file, 0, LV_FS_SEEK_SET);

    char* buff = lv_malloc(file_size);
    lv_fs_read(&file, buff, file_size, NULL);

    const char* json_string = buff;

    cJSON* json = cJSON_Parse(json_string);
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            LV_LOG_ERROR("parse error: %p", error_ptr);
        }
        lv_free(buff);
        return;
    }

    cJSON* musics_object = cJSON_GetObjectItem(json, "musics");

    if (musics_object == NULL) {
        lv_free(buff);
        return;
    }

    R.album_count = cJSON_GetArraySize(musics_object);
    R.albums = lv_malloc_zeroed(R.album_count * sizeof(album_info_t));

    for (int i = 0; i < R.album_count; i++) {
        cJSON* music_object = cJSON_GetArrayItem(musics_object, i);

        const char* path = cJSON_GetStringValue(cJSON_GetObjectItem(music_object, "path"));
        const char* name = cJSON_GetStringValue(cJSON_GetObjectItem(music_object, "name"));
        const char* artist = cJSON_GetStringValue(cJSON_GetObjectItem(music_object, "artist"));
        const char* cover = cJSON_GetStringValue(cJSON_GetObjectItem(music_object, "cover"));
        const double total_time_double = cJSON_GetNumberValue(cJSON_GetObjectItem(music_object, "total_time"));
        const char* color_str = cJSON_GetStringValue(cJSON_GetObjectItem(music_object, "color"));

        uint64_t total_time = (uint64_t)total_time_double;
        uint32_t color_int = strtoul(color_str + 1, NULL, 16);

        if (total_time == 0)
            total_time = 1;

        lv_color_t color = lv_color_hex(color_int);

        lv_snprintf(R.albums[i].path, sizeof(R.albums[i].path), "%s/%s", MUSICS_ROOT, path);
        lv_snprintf(R.albums[i].cover, sizeof(R.albums[i].cover), "%s/%s", MUSICS_ROOT, cover);
        R.albums[i].name = lv_strdup(name);
        R.albums[i].artist = lv_strdup(artist);
        R.albums[i].total_time = total_time;
        R.albums[i].color = color;

        LV_LOG_USER("Album %d: %s - %s | %s %s %llu", i, R.albums[i].name, R.albums[i].artist, R.albums[i].path, R.albums[i].cover, total_time);
    }

    cJSON_Delete(json);

    lv_free(buff);
}
