/**
 * @file dialplate.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "page.h"

#include "../hal/hal.h"

/*********************
 *      DEFINES
 *********************/

#define EVENT_CNT 1

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    uint32_t color;
    lv_coord_t y;
    lv_coord_t bar_width;
    lv_coord_t arc_radius;
    const void* img_src;
    lv_obj_t* label;
    lv_obj_t* bar;
    lv_obj_t* arc;
} color_bar_t;

enum color_bar_index {
    COLOR_BAR_WEATHER,
    COLOR_BAR_STEP,
    COLOR_BAR_HEART,
    COLOR_BAR_MAX
};

typedef struct {
    lv_fragment_t base;
    lv_span_t* span_date;
    lv_span_t* span_month;
    lv_span_t* span_week;
    lv_obj_t* bar_batt;
    lv_obj_t* label_time_hour;
    lv_obj_t* label_time_min;
    lv_timer_t* timer_label_time_update;
    lv_timer_t* timer_color_bar_update;
    clock_value_t clock_value;
    color_bar_t color_bar_grp[COLOR_BAR_MAX];
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
static lv_obj_t* obj_batt_create(lv_obj_t* par)
{
    lv_obj_t* bar = lv_obj_create(par);
    lv_obj_set_style_bg_color(bar, lv_color_white(), LV_PART_MAIN);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 0, LV_PART_MAIN);
    lv_obj_set_size(bar, lv_obj_get_self_width(par) / 2, lv_obj_get_self_height(par) - 2);
    lv_obj_align(bar, LV_ALIGN_LEFT_MID, 2, 0);

    return bar;
}

static void topbar_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* topbar = lv_obj_create(par);
    lv_obj_set_size(topbar, LV_PCT(100), 25);
    lv_obj_set_style_bg_color(topbar, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(topbar, 0, LV_PART_MAIN);
    lv_obj_clear_flag(topbar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(topbar, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t* img1 = lv_img_create(topbar);
    lv_img_set_src(img1, resource_get_img("icon_bluetooth"));
    lv_obj_align(img1, LV_ALIGN_LEFT_MID, 20, 0);

    lv_obj_t* img2 = lv_img_create(topbar);
    lv_img_set_src(img2, resource_get_img("icon_battery"));
    lv_obj_set_style_pad_all(topbar, 0, LV_PART_MAIN);
    lv_obj_align(img2, LV_ALIGN_RIGHT_MID, -20, 0);

    lv_obj_t* bar = obj_batt_create(img2);
    ctx->bar_batt = bar;
}

static void label_date_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* spangroup = lv_spangroup_create(par);
    lv_obj_set_style_text_font(spangroup, resource_get_font(BANDX_BOLD_FONT "_23"), LV_PART_MAIN);
    lv_obj_set_style_text_color(spangroup, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(spangroup, LV_ALIGN_TOP_MID, 0, 25);

    lv_span_t* span1 = lv_spangroup_new_span(spangroup);
    lv_span_t* span2 = lv_spangroup_new_span(spangroup);
    lv_span_t* span3 = lv_spangroup_new_span(spangroup);
    lv_style_set_text_color(&span2->style, lv_color_hex(0xF15A24));

    ctx->span_month = span1;
    ctx->span_date = span2;
    ctx->span_week = span3;
}

static void label_time_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* label = lv_label_create(par);
    lv_obj_set_style_text_font(label, resource_get_font(BANDX_BOLD_FONT "_110"), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 33);
    ctx->label_time_hour = label;

    lv_obj_t* img = lv_img_create(par);
    lv_img_set_src(img, resource_get_img("num_shadow"));
    lv_obj_set_pos(img, 16, 136);

    img = lv_img_create(par);
    lv_img_set_src(img, resource_get_img("num_shadow"));
    lv_obj_set_pos(img, 99, 136);

    label = lv_label_create(par);
    lv_obj_set_style_text_font(label, resource_get_font(BANDX_BOLD_FONT "_110"), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xF15A24), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 115);
    ctx->label_time_min = label;
}

static void label_time_update(lv_timer_t* timer)
{
    page_ctx_t* ctx = lv_timer_get_user_data(timer);
    clock_get_value(&ctx->clock_value);

    char month_buf[32];
    char date_buf[32];
    const char* week_str[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
    int week_index = ctx->clock_value.week % ARRAY_SIZE(week_str);

    lv_snprintf(month_buf, sizeof(month_buf), "%02d-", ctx->clock_value.month);
    lv_snprintf(date_buf, sizeof(date_buf), "%02d ", ctx->clock_value.date);
    lv_span_set_text(ctx->span_month, month_buf);
    lv_span_set_text(ctx->span_date, date_buf);
    lv_span_set_text(ctx->span_week, week_str[week_index]);

    lv_label_set_text_fmt(ctx->label_time_hour, "%02d", ctx->clock_value.hour);
    lv_label_set_text_fmt(ctx->label_time_min, "%02d", ctx->clock_value.min);
}

static void color_bar_create(lv_obj_t* par, color_bar_t* color_bar, int len)
{
    for (int i = 0; i < len; i++) {
        lv_color_t color = lv_color_hex(color_bar[i].color);
        lv_coord_t y_pos = color_bar[i].y;
        lv_coord_t bar_width = color_bar[i].bar_width;
        lv_coord_t arc_size = color_bar[i].arc_radius * 2;

        lv_obj_t* bar = lv_bar_create(par);
        lv_bar_set_value(bar, 100, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(bar, color, LV_PART_MAIN);
        lv_obj_set_size(bar, bar_width, 32);
        lv_obj_set_pos(bar, -12, y_pos);

        lv_obj_t* arc = lv_arc_create(par);
        lv_obj_set_style_arc_color(arc, color, LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(
            arc,
            lv_obj_get_style_height(bar, 0),
            LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(arc, color, LV_PART_KNOB);
        lv_obj_set_style_pad_all(arc, 0, LV_PART_KNOB);
        lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(arc, 0, LV_PART_MAIN);
        lv_arc_set_angles(arc, 270, 360);
        lv_arc_set_bg_angles(arc, 270, 360);
        lv_obj_set_size(arc, arc_size, arc_size);
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_align_to(arc, bar, LV_ALIGN_OUT_RIGHT_TOP, -arc_size / 2 - 10, 0);

        lv_obj_t* img = lv_img_create(bar);
        lv_img_set_src(img, resource_get_img(color_bar[i].img_src));
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 20, 0);

        if (i == COLOR_BAR_WEATHER) {
            lv_obj_t* img_deg = lv_img_create(bar);
            lv_img_set_src(img_deg, resource_get_img("centigrade"));
            lv_obj_align(img_deg, LV_ALIGN_RIGHT_MID, 0, 0);
        }

        lv_obj_t* label = lv_label_create(bar);
        lv_obj_set_style_text_font(
            label,
            resource_get_font(BANDX_BOLD_FONT "_28"),
            LV_PART_MAIN);
        lv_obj_set_style_text_color(
            label,
            lv_color_white(),
            LV_PART_MAIN);
        lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

        lv_obj_move_foreground(bar);

        color_bar[i].bar = bar;
        color_bar[i].label = label;
        color_bar[i].arc = arc;
    }
}

static void color_bar_update(lv_timer_t* timer)
{
    page_ctx_t* ctx = lv_timer_get_user_data(timer);

    lv_label_set_text_fmt(ctx->color_bar_grp[COLOR_BAR_WEATHER].label, "15");
    lv_label_set_text_fmt(ctx->color_bar_grp[COLOR_BAR_STEP].label, "%d", imu_get_steps());
    lv_label_set_text_fmt(
        ctx->color_bar_grp[COLOR_BAR_HEART].label,
        "%d",
        (int)particle_sensor_get_beats());
}

static void auto_event_create(page_ctx_t* ctx)
{
    lv_auto_event_data_t ae_grp[EVENT_CNT] = {
        { &ctx->base.obj, LV_EVENT_LEAVE, 2000 },
    };
    for (int i = 0; i < EVENT_CNT; i++) {
        ctx->event_data[i] = ae_grp[i];
    }
    AUTO_EVENT_CREATE(ctx->auto_event, ctx->event_data);
}

static void on_root_event(lv_event_t* e)
{
    lv_obj_t* root = lv_event_get_current_target_obj(e);
    lv_event_code_t code = lv_event_get_code(e);
    page_ctx_t* ctx = lv_obj_get_user_data(root);

    if (code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT) {
            lv_obj_send_event(root, LV_EVENT_LEAVE, NULL);
        }
    } else if (code == LV_EVENT_LEAVE) {
        page_push(&ctx->base, "launcher", NULL);
        page_set_last_page_dialplate(true);
    }
}

static void on_page_construct(lv_fragment_t* self, void* args)
{
    LV_LOG_INFO("self: %p args: %p", self, args);

    page_ctx_t* ctx = (page_ctx_t*)self;

    color_bar_t color_bar_grp[COLOR_BAR_MAX] = {
        { .color = 0xF7931E,
            .y = 245,
            .bar_width = 130,
            .arc_radius = 120,
            .img_src = "weather" },

        { .color = 0x3FA9F5,
            .y = 285,
            .bar_width = 130,
            .arc_radius = 80,
            .img_src = "step" },

        { .color = 0xED1C24,
            .y = 325,
            .bar_width = 130,
            .arc_radius = 40,
            .img_src = "heart" },
    };

    for (int i = 0; i < COLOR_BAR_MAX; i++) {
        ctx->color_bar_grp[i] = color_bar_grp[i];
    }
}

static void on_page_destruct(lv_fragment_t* self)
{
    LV_LOG_INFO("self: %p", self);
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
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    clock_get_value(&ctx->clock_value);

    topbar_create(obj);
    label_date_create(obj);
    label_time_create(obj);
    color_bar_create(obj, ctx->color_bar_grp, COLOR_BAR_MAX);

    ctx->timer_label_time_update = lv_timer_create(label_time_update, 100, ctx);
    label_time_update(ctx->timer_label_time_update);

    ctx->timer_color_bar_update = lv_timer_create(color_bar_update, 1000, ctx);
    color_bar_update(ctx->timer_color_bar_update);
    auto_event_create(ctx);
}

static void on_page_will_delete(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);

    page_ctx_t* ctx = (page_ctx_t*)self;
    lv_timer_del(ctx->timer_color_bar_update);
    lv_timer_del(ctx->timer_label_time_update);
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

PAGE_CLASS_DEF(dialplate);
