/**
 * @file music.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "page.h"

/*********************
 *      DEFINES
 *********************/

#define MC_COLOR_GRAY lv_color_make(0x33, 0x33, 0x33)
#define MC_COLOR_PINK lv_color_make(0xB9, 0x00, 0x5E)
#define MC_COLOR_PINK_BG lv_color_make(0x17, 0x00, 0x0B)

#define MUSIC_TIME_TO_MS(min, sec) ((min)*60 * 1000 + (sec)*1000)
#define MUSIC_INFO_CNT 3
#define EVENT_CNT 9

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    const char* name;
    uint32_t time_ms;
} music_info_t;

typedef struct {
    lv_fragment_t base;
    lv_style_t btn_style;
    lv_style_t trans_style;
    lv_obj_t* bar_volume;
    lv_obj_t* arc_play;
    lv_obj_t* obj_play;
    lv_obj_t* label_music_info;
    lv_obj_t* btn_volume_add;
    lv_obj_t* btn_volume_reduce;
    lv_obj_t* btn_music_ctrl_next;
    lv_obj_t* btn_music_ctrl_prev;
    lv_span_t* span_cur_time;
    lv_span_t* span_end_time;
    lv_timer_t* timer_music_update;
    uint32_t music_current_time;
    uint32_t music_end_time;
    uint16_t music_current_index;
    bool music_is_playing;
    music_info_t music_info_grp[MUSIC_INFO_CNT];
    lv_auto_event_data_t event_data[EVENT_CNT];
    lv_auto_event_t* auto_event;
} page_ctx_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void on_root_event(lv_event_t* e)
{
    lv_obj_t* root = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    page_ctx_t* ctx = lv_obj_get_user_data(root);

    if (code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_RIGHT) {
            lv_obj_send_event(root, LV_EVENT_LEAVE, NULL);
        }
    } else if (code == LV_EVENT_LEAVE) {
        page_pop(&ctx->base);
    }
}

static void bar_volume_set_value(int16_t value, lv_obj_t* bar_volume)
{
    lv_bar_set_value(bar_volume, value, LV_ANIM_ON);
}

static void bar_volume_create(lv_obj_t* par)
{
    page_ctx_t* ctx = lv_obj_get_user_data(par);

    lv_obj_t* bar = lv_bar_create(par);
    lv_obj_set_size(bar, 160, 3);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 70);
    lv_obj_set_style_bg_color(bar, MC_COLOR_PINK, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, MC_COLOR_PINK, LV_PART_INDICATOR);

    ctx->bar_volume = bar;
}

static void btn_volume_event_handler(lv_event_t* event)
{
    lv_obj_t* obj = lv_event_get_target(event);
    lv_event_code_t code = lv_event_get_code(event);
    page_ctx_t* ctx = lv_event_get_user_data(event);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* img = lv_obj_get_child(obj, 0);
        int16_t vol_value = lv_bar_get_value(ctx->bar_volume);

        if (lv_strcmp(lv_img_get_src(img), resource_get_img("icon_volume_add")) == 0) {
            bar_volume_set_value(vol_value + 10, ctx->bar_volume);
        } else {
            bar_volume_set_value(vol_value - 10, ctx->bar_volume);
        }
    }
}

static void btn_volume_create(lv_obj_t* par)
{
    page_ctx_t* ctx = lv_obj_get_user_data(par);

    lv_obj_t* btn1 = lv_btn_create(par);
    lv_obj_set_size(btn1, 85, 48);
    lv_obj_add_style(btn1, &ctx->btn_style, LV_PART_MAIN);
    lv_obj_add_style(btn1, &ctx->trans_style, LV_STATE_DEFAULT);
    lv_obj_add_style(btn1, &ctx->trans_style, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(btn1, lv_color_white(), LV_STATE_PRESSED);
    lv_obj_align(btn1, LV_ALIGN_TOP_MID, -lv_obj_get_style_width(btn1, 0) / 2 - 3, 10);
    lv_obj_add_event(btn1, btn_volume_event_handler, LV_EVENT_ALL, ctx);

    lv_obj_t* btn2 = lv_btn_create(par);
    lv_obj_set_size(btn2, 85, 48);
    lv_obj_add_style(btn2, &ctx->btn_style, LV_PART_MAIN);
    lv_obj_add_style(btn2, &ctx->trans_style, LV_STATE_DEFAULT);
    lv_obj_add_style(btn2, &ctx->trans_style, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(btn2, lv_color_white(), LV_STATE_PRESSED);
    lv_obj_align(btn2, LV_ALIGN_TOP_MID, lv_obj_get_style_width(btn2, 0) / 2 + 3, 10);
    lv_obj_add_event(btn2, btn_volume_event_handler, LV_EVENT_ALL, ctx);

    lv_obj_t* img1 = lv_img_create(btn1);
    lv_img_set_src(img1, resource_get_img("icon_volume_reduce"));
    lv_obj_set_style_image_recolor_opa(img1, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_image_recolor(img1, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* img2 = lv_img_create(btn2);
    lv_img_set_src(img2, resource_get_img("icon_volume_add"));
    lv_obj_set_style_image_recolor_opa(img2, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_image_recolor(img2, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(img2, LV_ALIGN_CENTER, 0, 0);

    ctx->btn_volume_reduce = btn1;
    ctx->btn_volume_add = btn2;
}

static void obj_play_anim_ready_callback(lv_anim_t* a_p)
{
    page_ctx_t* ctx = lv_anim_get_user_data(a_p);

    if (a_p->act_time == 0)
        return;

    lv_obj_t* img = (lv_obj_t*)a_p->var;
    const char* img_src_next;
    img_src_next = (lv_strcmp(lv_img_get_src(img), resource_get_img("icon_start")) == 0) ? resource_get_img("icon_pause") : resource_get_img("icon_start");
    lv_img_set_src(img, img_src_next);

    ctx->music_is_playing = (lv_strcmp(lv_img_get_src(img), resource_get_img("icon_pause")) == 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_angle);
    lv_anim_set_values(&a, 1800, 3600);
    lv_anim_set_time(&a, 300);

    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

static void obj_play_event_handler(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* obj = lv_event_get_target(event);
    page_ctx_t* ctx = lv_event_get_user_data(event);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* img = lv_obj_get_child(obj, 0);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_angle);
        lv_anim_set_ready_cb(&a, obj_play_anim_ready_callback);
        lv_anim_set_user_data(&a, ctx);
        lv_anim_set_values(&a, 0, 1800);
        lv_anim_set_time(&a, 300);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
        lv_anim_start(&a);
    }
}

static void obj_play_create(lv_obj_t* par)
{
    page_ctx_t* ctx = lv_obj_get_user_data(par);

    lv_obj_t* obj = lv_obj_create(ctx->arc_play);
    lv_obj_set_size(obj, 104, 104);
    lv_obj_add_style(obj, &ctx->btn_style, LV_PART_MAIN);
    lv_obj_add_style(obj, &ctx->trans_style, LV_STATE_DEFAULT);
    lv_obj_add_style(obj, &ctx->trans_style, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, lv_color_white(), LV_STATE_PRESSED);
    lv_obj_add_event(obj, obj_play_event_handler, LV_EVENT_ALL, ctx);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, MC_COLOR_GRAY, LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* img = lv_img_create(obj);
    lv_img_set_src(img, resource_get_img("icon_start"));
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    ctx->obj_play = obj;
}

static void spangroup_time_update(page_ctx_t* ctx, uint32_t cur_ms, uint32_t end_ms)
{
    uint32_t cur_min = cur_ms / 60 / 1000;
    uint32_t cur_sec = (cur_ms / 1000) % 60;

    uint32_t end_min = end_ms / 60 / 1000;
    uint32_t end_sec = (end_ms / 1000) % 60;

    char cur_buf[32];
    char end_buf[32];

    lv_snprintf(cur_buf, sizeof(cur_buf), "%d:%02d", cur_min, cur_sec);
    lv_snprintf(end_buf, sizeof(end_buf), " -- %d:%02d", end_min, end_sec);
    lv_span_set_text(ctx->span_cur_time, cur_buf);
    lv_span_set_text(ctx->span_end_time, end_buf);
}

static void arc_play_event_handler(lv_event_t* event)
{
    page_ctx_t* ctx = lv_event_get_user_data(event);
    int32_t value = lv_arc_get_value(ctx->arc_play);
    ctx->music_current_time = lv_map(value, lv_arc_get_min_value(ctx->arc_play), lv_arc_get_max_value(ctx->arc_play), 0, ctx->music_end_time);
    spangroup_time_update(ctx, ctx->music_current_time, ctx->music_end_time);
}

static void arc_play_create(lv_obj_t* par)
{
    page_ctx_t* ctx = lv_obj_get_user_data(par);

    lv_obj_t* arc = lv_arc_create(par);
    lv_obj_set_style_arc_color(arc, MC_COLOR_PINK, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 6, LV_PART_INDICATOR);

    lv_obj_set_style_bg_color(arc, MC_COLOR_PINK, LV_PART_KNOB); // 设置 arc knob颜色
    lv_obj_set_style_pad_all(arc, 0, LV_PART_KNOB); // 设置arc knob size

    lv_obj_set_style_arc_color(arc, MC_COLOR_PINK_BG, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 6, LV_PART_MAIN);

    lv_obj_set_style_border_width(arc, 0, LV_PART_MAIN);
    lv_obj_set_size(arc, 121, 121);
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(arc, arc_play_event_handler, LV_EVENT_VALUE_CHANGED, ctx);

    lv_arc_set_start_angle(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_range(arc, 0, 1000);
    lv_arc_set_rotation(arc, 270);

    ctx->arc_play = arc;
}

static void spangroup_time_create(lv_obj_t* par)
{
    page_ctx_t* ctx = lv_obj_get_user_data(par);

    lv_obj_t* label = lv_label_create(par);
    lv_obj_set_style_text_font(label, resource_get_font("bahnschrift_20"), LV_PART_MAIN);
    lv_label_set_text(label, "--");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 87);

    lv_obj_t* spangroup = lv_spangroup_create(par);
    lv_obj_set_style_text_font(spangroup, resource_get_font("bahnschrift_20"), LV_PART_MAIN);

    lv_span_t* cur_span = lv_spangroup_new_span(spangroup);
    lv_style_set_text_color(&cur_span->style, lv_color_hex(0xB9005E));

    lv_span_t* end_span = lv_spangroup_new_span(spangroup);
    lv_style_set_text_color(&end_span->style, lv_color_hex(0x666666));
    lv_span_set_text(end_span, "--");

    lv_obj_align(spangroup, LV_ALIGN_TOP_MID, 0, 87);

    ctx->span_cur_time = cur_span;
    ctx->span_end_time = end_span;
}

static void label_music_info_create(lv_obj_t* par)
{
    page_ctx_t* ctx = lv_obj_get_user_data(par);

    lv_obj_t* label = lv_label_create(par);
    lv_obj_set_style_text_font(label, resource_get_font("bahnschrift_20"), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(label, "");
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_obj_set_width(label, 166);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 259);

    ctx->label_music_info = label;
}

static void music_change_current(page_ctx_t* ctx, int index)
{
    index %= MUSIC_INFO_CNT;

    ctx->music_current_time = 0;
    ctx->music_end_time = ctx->music_info_grp[index].time_ms;

    lv_label_set_text(ctx->label_music_info, ctx->music_info_grp[index].name);
    lv_arc_set_value(ctx->arc_play, ctx->music_current_time * 1000 / ctx->music_end_time);
    spangroup_time_update(ctx, ctx->music_current_time, ctx->music_end_time);
}

static void music_change_next(page_ctx_t* ctx, bool next)
{
    if (next) {
        if (ctx->music_current_index >= MUSIC_INFO_CNT - 1) {
            ctx->music_current_index = 0;
        } else {
            ctx->music_current_index++;
        }
    } else {
        if (ctx->music_current_index == 0) {
            ctx->music_current_index = MUSIC_INFO_CNT;
        } else {
            ctx->music_current_index--;
        }
    }

    music_change_current(ctx, ctx->music_current_index);
}

static void btn_music_event_handler(lv_event_t* event)
{
    lv_obj_t* obj = lv_event_get_target(event);
    lv_event_code_t code = lv_event_get_code(event);
    page_ctx_t* ctx = lv_event_get_user_data(event);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* img = lv_obj_get_child(obj, 0);

        bool is_next = (lv_strcmp(lv_img_get_src(img), resource_get_img("icon_next")) == 0);
        music_change_next(ctx, is_next);
    }
}

static void btn_music_ctrl_create(lv_obj_t* par)
{
    page_ctx_t* ctx = lv_obj_get_user_data(par);

    lv_obj_t* btn1 = lv_btn_create(par);
    lv_obj_set_size(btn1, 85, 64);
    lv_obj_add_style(btn1, &ctx->btn_style, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn1, lv_color_white(), LV_STATE_PRESSED);
    lv_obj_add_style(btn1, &ctx->trans_style, LV_STATE_DEFAULT);
    lv_obj_add_style(btn1, &ctx->trans_style, LV_STATE_PRESSED);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_MID, -lv_obj_get_style_width(btn1, 0) / 2 - 3, -10);
    lv_obj_add_event(btn1, btn_music_event_handler, LV_EVENT_ALL, ctx);

    lv_obj_t* btn2 = lv_btn_create(par);
    lv_obj_set_size(btn2, 85, 64);
    lv_obj_add_style(btn2, &ctx->btn_style, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn2, lv_color_white(), LV_STATE_PRESSED);
    lv_obj_add_style(btn2, &ctx->trans_style, LV_STATE_DEFAULT);
    lv_obj_add_style(btn2, &ctx->trans_style, LV_STATE_PRESSED);
    lv_obj_add_event(btn2, btn_music_event_handler, LV_EVENT_ALL, ctx);
    lv_obj_align(btn2, LV_ALIGN_BOTTOM_MID, lv_obj_get_style_width(btn2, 0) / 2 + 3, -10);

    lv_obj_t* img1 = lv_img_create(btn1);
    lv_img_set_src(img1, resource_get_img("icon_prev"));
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* img2 = lv_img_create(btn2);
    lv_img_set_src(img2, resource_get_img("icon_next"));
    lv_obj_align(img2, LV_ALIGN_CENTER, 0, 0);

    ctx->btn_music_ctrl_prev = btn1;
    ctx->btn_music_ctrl_next = btn2;
}

static void music_update(lv_timer_t* timer)
{
    page_ctx_t* ctx = lv_timer_get_user_data(timer);

    if (!ctx->music_is_playing) {
        return;
    }
    ctx->music_current_time += timer->period;

    if (ctx->music_current_time > ctx->music_end_time) {
        music_change_next(ctx, true);
    }

    lv_arc_set_value(ctx->arc_play, ctx->music_current_time * 1000 / ctx->music_end_time);
    spangroup_time_update(ctx, ctx->music_current_time, ctx->music_end_time);
}

static void auto_event_create(page_ctx_t* ctx)
{
    lv_auto_event_data_t ae_grp[EVENT_CNT] = {
        { &ctx->obj_play, LV_EVENT_CLICKED, 1000 },
        { &ctx->btn_volume_add, LV_EVENT_CLICKED, 500 },
        { &ctx->btn_volume_add, LV_EVENT_CLICKED, 500 },
        { &ctx->btn_volume_reduce, LV_EVENT_CLICKED, 500 },
        { &ctx->btn_volume_reduce, LV_EVENT_CLICKED, 500 },
        { &ctx->btn_music_ctrl_next, LV_EVENT_CLICKED, 2000 },
        { &ctx->btn_music_ctrl_next, LV_EVENT_CLICKED, 2000 },
        { &ctx->obj_play, LV_EVENT_CLICKED, 1000 },
        { &ctx->base.obj, LV_EVENT_LEAVE, 1000 },
    };
    for (int i = 0; i < EVENT_CNT; i++) {
        ctx->event_data[i] = ae_grp[i];
    }
    AUTO_EVENT_CREATE(ctx->auto_event, ctx->event_data);
}

static void on_page_construct(lv_fragment_t* self, void* args)
{
    LV_LOG_INFO("self: %p args: %p", self, args);

    page_ctx_t* ctx = (page_ctx_t*)self;

    lv_style_init(&ctx->btn_style);
    lv_style_set_bg_color(&ctx->btn_style, MC_COLOR_GRAY);
    lv_style_set_border_width(&ctx->btn_style, 0);
    lv_style_set_outline_width(&ctx->btn_style, 0);
    lv_style_set_radius(&ctx->btn_style, 10);

    lv_style_init(&ctx->trans_style);
    static lv_style_transition_dsc_t dsc;
    static const lv_style_prop_t props[] = {
        LV_STYLE_BG_COLOR,
        LV_STYLE_TRANSFORM_WIDTH,
        LV_STYLE_TRANSFORM_HEIGHT,
        LV_STYLE_PROP_INV
    };
    lv_style_transition_dsc_init(
        &dsc,
        props,
        lv_anim_path_ease_in_out,
        50,
        0,
        NULL);
    lv_style_set_transition(&ctx->trans_style, &dsc);

    music_info_t music_info_grp[MUSIC_INFO_CNT] = {
        { .name = "Wannabe (Instrumental)", MUSIC_TIME_TO_MS(3, 37) },
        { .name = "River", MUSIC_TIME_TO_MS(3, 40) },
        { .name = "I Feel Tired", MUSIC_TIME_TO_MS(3, 25) },
    };

    for (int i = 0; i < MUSIC_INFO_CNT; i++) {
        ctx->music_info_grp[i] = music_info_grp[i];
    }

    ctx->music_current_time = 0;
    ctx->music_end_time = 0;
    ctx->music_current_index = 0;
    ctx->music_is_playing = false;
}

static void on_page_destruct(lv_fragment_t* self)
{
    LV_LOG_INFO("self: %p", self);

    page_ctx_t* ctx = (page_ctx_t*)self;
    lv_style_reset(&ctx->btn_style);
    lv_style_reset(&ctx->trans_style);
}

static void on_page_attached(lv_fragment_t* self)
{
    LV_LOG_INFO("self: %p", self);
}

static void on_page_detached(lv_fragment_t* self)
{
    LV_LOG_INFO("self: %p", self);
}

static lv_obj_t* on_page_create(lv_fragment_t* self, lv_obj_t* container)
{
    LV_LOG_INFO("self: %p container: %p", self, container);

    lv_obj_t* root = lv_obj_create(container);
    lv_obj_remove_style_all(root);
    lv_obj_add_style(root, resource_get_style("root_def"), 0);
    lv_obj_add_event(root, on_root_event, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_set_user_data(root, self);
    return root;
}

static void on_page_created(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);

    page_ctx_t* ctx = (page_ctx_t*)self;

    btn_volume_create(obj);
    bar_volume_create(obj);
    bar_volume_set_value(50, ctx->bar_volume);

    arc_play_create(obj);
    obj_play_create(obj);
    spangroup_time_create(obj);
    label_music_info_create(obj);
    btn_music_ctrl_create(obj);

    ctx->music_is_playing = false;
    music_change_current(ctx, ctx->music_current_index);
    auto_event_create(ctx);
    ctx->timer_music_update = lv_timer_create(music_update, 100, ctx);
}

static void on_page_will_delete(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);

    page_ctx_t* ctx = (page_ctx_t*)self;
    lv_timer_del(ctx->timer_music_update);
    AUTO_EVENT_DELETE(ctx->auto_event);
}

static void on_page_deleted(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);
}

static bool on_page_event(lv_fragment_t* self, int code, void* user_data)
{
    LV_LOG_INFO("self: %p code: %d user_data: %p", self, code, user_data);
    return false;
}

PAGE_CLASS_DEF(music);
