#include <lvgl/lvgl.h>
#include "wooden_fish.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

static void create_pr_animation(wooden_fish_t *app, int32_t x, int32_t y);
static void create_merit_animation(wooden_fish_t *app, int32_t x, int32_t y);
static void anim_ready_cb(lv_anim_t *a);
static void auto_knock_timer_cb(lv_timer_t *timer);

static wooden_fish_t g_wooden_fish;
static bool is_coder_mode = false;
static lv_obj_t *coder_switch = NULL;

static audioctl_s *g_audioctl = NULL;

static void play_wooden_fish_sound(void)
{
    static uint32_t last_play_time = 0;
    uint32_t current_time = lv_tick_get();

    if (current_time - last_play_time < 500) {
        return;
    }

    last_play_time = current_time;

    if (g_audioctl) {
        audio_ctl_stop(g_audioctl);
        audio_ctl_uninit_nxaudio(g_audioctl);
        g_audioctl = NULL;
    }

    g_audioctl = audio_ctl_init_nxaudio(WOODEN_FISH_SOUND_PATH);
    if (g_audioctl) {
        audio_ctl_start(g_audioctl);
    }
}

static void auto_knock_timer_cb(lv_timer_t *timer) {
    wooden_fish_t *app = (wooden_fish_t *)timer->user_data;
    if (!app) return;

    if (app->ui.auto_switch == NULL || !lv_obj_has_state(app->ui.auto_switch, LV_STATE_CHECKED)) {
        return;
    }

    app->knock_count++;

    if (is_coder_mode) {
        lv_label_set_text_fmt(app->ui.counter, "pr: %lu", (unsigned long)app->knock_count);
        lv_label_set_text_fmt(app->ui.today_label, "今日pr: %lu", (unsigned long)app->knock_count);
    } else {
        lv_label_set_text_fmt(app->ui.counter, "功德: %lu", (unsigned long)app->knock_count);
        lv_label_set_text_fmt(app->ui.today_label, "今日功德: %lu", (unsigned long)app->knock_count);
    }

    play_wooden_fish_sound();

    lv_anim_t anim = {0};
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, app->ui.fish_img);
    lv_anim_set_values(&anim, 0, 10);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_time(&anim, 100);
    lv_anim_set_playback_time(&anim, 100);
    lv_anim_start(&anim);

    lv_area_t fish_area;
    lv_obj_get_coords(app->ui.fish_img, &fish_area);
    int32_t center_x = fish_area.x1 + (fish_area.x2 - fish_area.x1) / 2;
    int32_t center_y = fish_area.y1 + (fish_area.y2 - fish_area.y1) / 2;

    if (is_coder_mode) {
        create_pr_animation(app, center_x, center_y);
    } else {
        create_merit_animation(app, center_x, center_y);
    }
}

static void fish_click_cb(lv_event_t *e) {
    wooden_fish_t *app = (wooden_fish_t *)lv_event_get_user_data(e);
    app->knock_count++;
    if (is_coder_mode) {
        lv_label_set_text_fmt(app->ui.counter, "pr: %lu", (unsigned long)app->knock_count);
        lv_label_set_text_fmt(app->ui.today_label, "今日pr: %lu", (unsigned long)app->knock_count);
    } else {
        lv_label_set_text_fmt(app->ui.counter, "功德: %lu", (unsigned long)app->knock_count);
        lv_label_set_text_fmt(app->ui.today_label, "今日功德: %lu", (unsigned long)app->knock_count);
    }

    play_wooden_fish_sound();

    lv_anim_t anim = {0};
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, app->ui.fish_img);
    lv_anim_set_values(&anim, 0, 10);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_angle);
    lv_anim_set_time(&anim, 100);
    lv_anim_set_playback_time(&anim, 100);
    lv_anim_start(&anim);
    lv_point_t point;
    lv_indev_t *indev = lv_indev_active();
    if (indev != NULL) {
        lv_indev_get_point(indev, &point);
        if (is_coder_mode) {
            create_pr_animation(app, point.x, point.y);
        } else {
            create_merit_animation(app, point.x, point.y);
        }
    }
}

static void auto_switch_cb(lv_event_t *e) {
    lv_obj_t *sw = lv_event_get_target(e);
    wooden_fish_t *app = (wooden_fish_t *)lv_event_get_user_data(e);

    if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        app->auto_timer = lv_timer_create(auto_knock_timer_cb, 2000, app);
    } else {
        if (app->auto_timer) {
            lv_timer_del(app->auto_timer);
            app->auto_timer = NULL;
        }
    }
}

static void switch_mode_cb(lv_event_t *e) {
    lv_obj_t *sw = lv_event_get_target(e);
    is_coder_mode = lv_obj_has_state(sw, LV_STATE_CHECKED);
    if (is_coder_mode) {
        LV_LOG_USER("切换图片路径: %s", WOODEN_FISH_ENTER_IMG_PATH);
        lv_img_set_src(g_wooden_fish.ui.fish_img, WOODEN_FISH_ENTER_IMG_PATH);
        lv_img_set_zoom(g_wooden_fish.ui.fish_img, 400);
        lv_obj_invalidate(g_wooden_fish.ui.fish_img);
        lv_label_set_text(g_wooden_fish.ui.today_label, "今日pr: 0");
        lv_label_set_text(g_wooden_fish.ui.counter, "pr: 0");
    } else {
        LV_LOG_USER("切换图片路径: %s", WOODEN_FISH_IMG_PATH);
        lv_img_set_src(g_wooden_fish.ui.fish_img, WOODEN_FISH_IMG_PATH);
        lv_img_set_zoom(g_wooden_fish.ui.fish_img, 250);
        lv_obj_invalidate(g_wooden_fish.ui.fish_img);
        lv_label_set_text(g_wooden_fish.ui.today_label, "今日功德: 0");
        lv_label_set_text(g_wooden_fish.ui.counter, "功德: 0");
    }
    g_wooden_fish.knock_count = 0;
}

static void create_pr_animation(wooden_fish_t *app, int32_t x, int32_t y) {
    if (app->font == NULL) return;
    lv_obj_t *label = lv_label_create(lv_layer_top());
    if (!label) return;
    lv_label_set_text(label, "pr+1");
    lv_obj_set_style_text_font(label, app->font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x00BFFF), 0);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(label, 0, 0);
    lv_point_t text_size;
    lv_text_get_size(&text_size, "pr+1", app->font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    int32_t label_x = x - text_size.x / 2;
    int32_t label_y = y - text_size.y / 2;
    lv_obj_set_size(label, text_size.x, text_size.y);
    lv_obj_set_pos(label, label_x, label_y);
    lv_anim_t move_anim = {0};
    lv_anim_init(&move_anim);
    lv_anim_set_var(&move_anim, label);
    lv_anim_set_values(&move_anim, label_y, label_y - 120);
    lv_anim_set_exec_cb(&move_anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_time(&move_anim, 1500);
    lv_anim_set_path_cb(&move_anim, lv_anim_path_ease_out);
    lv_anim_t fade_anim = {0};
    lv_anim_init(&fade_anim);
    lv_anim_set_var(&fade_anim, label);
    lv_anim_set_values(&fade_anim, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_exec_cb(&fade_anim, (lv_anim_exec_xcb_t)lv_obj_set_style_text_opa);
    lv_anim_set_time(&fade_anim, 1500);
    lv_anim_set_path_cb(&fade_anim, lv_anim_path_ease_out);
    lv_anim_set_ready_cb(&move_anim, anim_ready_cb);
    lv_anim_start(&move_anim);
    lv_anim_start(&fade_anim);
    lv_refr_now(NULL);
}

static void create_main_ui(void) {
    g_wooden_fish.ui.screen = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(g_wooden_fish.ui.screen);
    lv_obj_set_size(g_wooden_fish.ui.screen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(g_wooden_fish.ui.screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(g_wooden_fish.ui.screen, LV_OPA_COVER, 0);

    coder_switch = lv_switch_create(g_wooden_fish.ui.screen);
    lv_obj_align(coder_switch, LV_ALIGN_TOP_RIGHT, -20, 20);
    lv_obj_add_event_cb(coder_switch, switch_mode_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_t *coder_label = lv_label_create(g_wooden_fish.ui.screen);
    lv_label_set_text(coder_label, "程序猿模式");
    lv_obj_set_style_text_font(coder_label, g_wooden_fish.font, 0);
    lv_obj_set_style_text_color(coder_label, lv_color_white(), 0);
    lv_obj_align_to(coder_label, coder_switch, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    g_wooden_fish.ui.fish_img = lv_img_create(g_wooden_fish.ui.screen);
    lv_img_set_src(g_wooden_fish.ui.fish_img, WOODEN_FISH_IMG_PATH);
    lv_obj_align(g_wooden_fish.ui.fish_img, LV_ALIGN_CENTER, 0, -50);
    lv_obj_add_flag(g_wooden_fish.ui.fish_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(g_wooden_fish.ui.fish_img, fish_click_cb, LV_EVENT_CLICKED, &g_wooden_fish);
    lv_img_set_zoom(g_wooden_fish.ui.fish_img, 250);

    g_wooden_fish.ui.counter = lv_label_create(g_wooden_fish.ui.screen);
    lv_label_set_text(g_wooden_fish.ui.counter, "功德: 0");
    lv_obj_set_style_text_font(g_wooden_fish.ui.counter, g_wooden_fish.font, 0);
    lv_obj_align(g_wooden_fish.ui.counter, LV_ALIGN_CENTER, 0, 80);

    g_wooden_fish.ui.today_label = lv_label_create(g_wooden_fish.ui.screen);
    lv_label_set_text_fmt(g_wooden_fish.ui.today_label, "今日功德: %lu", (unsigned long)g_wooden_fish.knock_count);
    lv_font_t *big_font = lv_freetype_font_create(FONTS_ROOT "/MiSans-Normal.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 32, LV_FREETYPE_FONT_STYLE_NORMAL);
    if (big_font) {
        lv_obj_set_style_text_font(g_wooden_fish.ui.today_label, big_font, 0);
    } else {
        lv_obj_set_style_text_font(g_wooden_fish.ui.today_label, g_wooden_fish.font, 0);
    }
    lv_obj_set_style_text_color(g_wooden_fish.ui.today_label, lv_color_white(), 0);
    lv_obj_align(g_wooden_fish.ui.today_label, LV_ALIGN_TOP_MID, 0, 20);

    g_wooden_fish.ui.auto_switch = lv_switch_create(g_wooden_fish.ui.screen);
    lv_obj_align(g_wooden_fish.ui.auto_switch, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_add_event_cb(g_wooden_fish.ui.auto_switch, auto_switch_cb, LV_EVENT_VALUE_CHANGED, &g_wooden_fish);

    lv_obj_t *auto_label = lv_label_create(g_wooden_fish.ui.screen);
    lv_label_set_text(auto_label, "自动敲击");
    lv_obj_set_style_text_font(auto_label, g_wooden_fish.font, 0);
    lv_obj_set_style_text_color(auto_label, lv_color_white(), 0);
    lv_obj_align_to(auto_label, g_wooden_fish.ui.auto_switch, LV_ALIGN_OUT_LEFT_MID, -10, 0);
}

void wooden_fish_cleanup(void)
{
    if (g_audioctl) {
        audio_ctl_stop(g_audioctl);
        audio_ctl_uninit_nxaudio(g_audioctl);
        g_audioctl = NULL;
    }

    if (g_wooden_fish.auto_timer) {
        lv_timer_delete(g_wooden_fish.auto_timer);
        g_wooden_fish.auto_timer = NULL;
    }
}

void wooden_fish_app_create(void) {
    g_wooden_fish.knock_count = 0;
    g_wooden_fish.auto_timer = NULL;

    g_wooden_fish.font = lv_freetype_font_create(
        FONTS_ROOT "/MiSans-Normal.ttf",
        LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 
        24, 
        LV_FREETYPE_FONT_STYLE_NORMAL
    );

    if (g_wooden_fish.font == NULL) {

        LV_LOG_ERROR("字体加载失败: %s", FONTS_ROOT "/MiSans-Normal.ttf");
        g_wooden_fish.font = &lv_font_montserrat_24;
    } else {
        LV_LOG_INFO("字体加载成功: %s", FONTS_ROOT "/MiSans-Normal.ttf");
    }

    create_main_ui();
}

__attribute__((unused)) static void app_create_error_page(void)
{
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "字体加载失败，请检查资源文件。");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_center(label);
}

static void anim_ready_cb(lv_anim_t *a) {
    lv_obj_t *obj = (lv_obj_t *)a->var;
    if(obj && lv_obj_is_valid(obj)) {
        lv_obj_del(obj);
    }
}

void wooden_fish_audio_complete_cb(void) {
    LV_LOG_USER("音频播放完成");
}

static void create_merit_animation(wooden_fish_t *app, int32_t x, int32_t y){
    if (app->font == NULL) {
        LV_LOG_ERROR("字体不可用，无法创建功德+1动画");
        return;
    }

    lv_obj_t *label = lv_label_create(lv_layer_top());
    if (label == NULL) {
        LV_LOG_ERROR("创建功德+1标签失败");
        return;
    }

    lv_label_set_text(label, "功德+1");
    lv_obj_set_style_text_font(label, app->font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFD700), 0);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(label, 0, 0);

    lv_point_t text_size;
    lv_text_get_size(&text_size, "功德+1", app->font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

    int32_t label_x = x - text_size.x / 2;
    int32_t label_y = y - text_size.y / 2;
    lv_obj_set_size(label, text_size.x, text_size.y);
    lv_obj_set_pos(label, label_x, label_y);

    LV_LOG_INFO("创建功德+1动画: 位置(%d,%d) 尺寸(%d,%d)", 
                label_x, label_y, text_size.x, text_size.y);

    lv_anim_t move_anim = {0};
    lv_anim_init(&move_anim);
    lv_anim_set_var(&move_anim, label);
    lv_anim_set_values(&move_anim, label_y, label_y - 120);
    lv_anim_set_exec_cb(&move_anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_time(&move_anim, 1500);
    lv_anim_set_path_cb(&move_anim, lv_anim_path_ease_out);
    lv_anim_t fade_anim = {0};
    lv_anim_init(&fade_anim);
    lv_anim_set_var(&fade_anim, label);
    lv_anim_set_values(&fade_anim, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_exec_cb(&fade_anim, (lv_anim_exec_xcb_t)lv_obj_set_style_text_opa);
    lv_anim_set_time(&fade_anim, 1500);
    lv_anim_set_path_cb(&fade_anim, lv_anim_path_ease_out);
    lv_anim_set_ready_cb(&move_anim, anim_ready_cb);
    lv_anim_start(&move_anim);
    lv_anim_start(&fade_anim);
    lv_refr_now(NULL);
    LV_LOG_INFO("功德+1动画已启动");
}

