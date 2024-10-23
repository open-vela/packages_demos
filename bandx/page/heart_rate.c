/**
 * @file heart_rate.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "page.h"

#include "../hal/hal.h"
#include "../utils/lv_obj_ext_func.h"

/*********************
 *      DEFINES
 *********************/

#define COLOR_BAR_CNT 4
#define EVENT_CNT 4

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    const char* text;
    int value;
    uint32_t color;
    lv_coord_t y;
} color_bar_t;

typedef struct {
    lv_fragment_t base;
    lv_obj_t* img_down;
    lv_obj_t* img_up;
    lv_obj_t* btn_rst;
    lv_obj_t* img_heart;
    lv_obj_t* chart_hr;
    lv_obj_t* cont_hr;
    lv_obj_t* label_hr_current;
    lv_obj_t* label_hr_record_range;
    lv_obj_t* label_hr_record_time;
    lv_chart_series_t* chart_hr_ser1;
    lv_timer_t* timer_label_hr_update;
    lv_style_t color_bar_style;
    lv_style_t label_hr_style1;
    lv_style_t label_hr_style2;
    color_bar_t color_bar_grp[COLOR_BAR_CNT];
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

#define CONT_HR_MOVE_DOWN(down, cont_hr, img_heart)            \
    do {                                                       \
        LV_OBJ_ADD_ANIM(                                       \
            cont_hr,                                           \
            y,                                                 \
            (down) ? -PAGE_VER_RES : 0, LV_ANIM_TIME_DEFAULT); \
        img_heart_anim_enable(img_heart, !down);               \
    } while (0)

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void label_hr_current_update(lv_timer_t* timer)
{
    page_ctx_t* ctx = lv_timer_get_user_data(timer);

    float hr_min, hr_max;
    int hr_beats = (int)particle_sensor_get_beats();
    if (!particle_sensor_get_beats_range(&hr_min, &hr_max)) {
        return;
    }

    lv_label_set_text_fmt(ctx->label_hr_current, "%d", hr_beats);
    lv_label_set_text_fmt(ctx->label_hr_record_range, "%d-%d", (int)hr_min, (int)hr_max);
    lv_chart_set_next_value(ctx->chart_hr, ctx->chart_hr_ser1, hr_beats);
}

static void img_heart_anim_enable(lv_obj_t* img_heart, bool en)
{
    if (en) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img_heart);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_zoom);
        lv_anim_set_values(&a, 256, (int)(256 * 0.7f));
        lv_anim_set_time(&a, 300);
        lv_anim_set_playback_time(&a, 300);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_repeat_delay(&a, 800);
        lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
        lv_anim_start(&a);
    } else {
        lv_anim_del(img_heart, (lv_anim_exec_xcb_t)lv_img_set_zoom);
    }
}

static void cont_hr_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* obj = lv_obj_create(par);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, PAGE_HOR_RES, PAGE_VER_RES * 2);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_user_data(obj, ctx);

    ctx->cont_hr = obj;
}

static void label_hr_current_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* img = lv_img_create(par);
    lv_img_set_src(img, resource_get_img("heart_color"));
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 28);

    lv_obj_t* label1 = lv_label_create(par);
    lv_obj_set_style_text_font(label1, resource_get_font(BANDX_REGULAR_FONT "_72"), LV_PART_MAIN);
    lv_label_set_text(label1, "--");
    lv_obj_set_style_text_color(label1, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label1, LV_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 95);

    lv_obj_t* label2 = lv_label_create(par);
    lv_obj_set_style_text_font(
        label2,
        resource_get_font(BANDX_REGULAR_FONT "_20"),
        LV_PART_MAIN);
    lv_obj_set_style_text_color(label2, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_label_set_text(label2, "bpm");
    lv_obj_align_to(label2, label1, LV_ALIGN_OUT_RIGHT_BOTTOM, 10, -20);

    ctx->label_hr_current = label1;
    ctx->img_heart = img;
}

static void label_hr_record_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* label1 = lv_label_create(par);
    lv_obj_add_style(label1, &ctx->label_hr_style1, LV_PART_MAIN);
    lv_label_set_text(label1, "Recent updates");
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 165);

    lv_obj_t* label2 = lv_label_create(par);
    lv_obj_add_style(label2, &ctx->label_hr_style2, LV_PART_MAIN);
    lv_label_set_text(label2, "--:--");
    lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 192);

    lv_obj_t* label3 = lv_label_create(par);
    lv_obj_add_style(label3, &ctx->label_hr_style1, LV_PART_MAIN);
    lv_label_set_text(label3, "Range");
    lv_obj_align(label3, LV_ALIGN_TOP_MID, 0, 222);

    lv_obj_t* label4 = lv_label_create(par);
    lv_obj_add_style(label4, &ctx->label_hr_style2, LV_PART_MAIN);
    lv_label_set_text(label4, "");
    lv_obj_align(label4, LV_ALIGN_TOP_MID, 0, 246);

    ctx->label_hr_record_time = label2;
    ctx->label_hr_record_range = label4;
}

static void label_hr_record_time_update(lv_obj_t* label)
{
    clock_value_t clock_value;
    clock_get_value(&clock_value);
    lv_label_set_text_fmt(label, "%02d:%02d", clock_value.hour, clock_value.min);
}

static void btn_rst_event_handler(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* obj = lv_event_get_current_target_obj(event);
    page_ctx_t* ctx = (page_ctx_t*)lv_event_get_user_data(event);

    if (code == LV_EVENT_CLICKED) {
        particle_sensor_reset_beats_range();
        label_hr_record_time_update(ctx->label_hr_record_time);

        lv_obj_t* img = lv_obj_get_child(obj, 0);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_angle);
        lv_anim_set_values(&a, 0, 3600);
        lv_anim_set_time(&a, 1000);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_start(&a);
    }
}

static void btn_rst_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* btn = lv_btn_create(par);
    lv_obj_set_size(btn, 149, 44);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 279);
    lv_obj_add_event(btn, btn_rst_event_handler, LV_EVENT_ALL, ctx);
    lv_obj_set_style_bg_color(
        btn,
        lv_color_make(0xFF, 0x54, 0x2A),
        LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(
        btn,
        lv_palette_main(LV_PALETTE_GREY),
        LV_STATE_PRESSED);

    lv_style_t* trans_style = resource_get_style("btn_trans");
    lv_obj_add_style(btn, trans_style, LV_STATE_PRESSED);
    lv_obj_add_style(btn, trans_style, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);

    lv_obj_t* img = lv_img_create(btn);
    lv_img_set_src(img, resource_get_img("icon_reset"));
    lv_obj_align_to(img, btn, LV_ALIGN_CENTER, 0, 0);

    ctx->btn_rst = btn;
}

static void img_arrow_event_handler(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* obj = lv_event_get_current_target_obj(event);
    page_ctx_t* ctx = (page_ctx_t*)lv_event_get_user_data(event);

    if (code == LV_EVENT_CLICKED) {
        bool is_down = (lv_strcmp(lv_img_get_src(obj), resource_get_img("arrow_down")) == 0);
        CONT_HR_MOVE_DOWN(is_down, ctx->cont_hr, ctx->img_heart);
    }
}

static void img_arrow_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* img1 = lv_img_create(par);
    lv_img_set_src(img1, resource_get_img("arrow_down"));
    lv_obj_align(img1, LV_ALIGN_TOP_MID, 0, 339);
    lv_obj_add_event(img1, img_arrow_event_handler, LV_EVENT_ALL, ctx);
    lv_obj_add_flag(img1, LV_OBJ_FLAG_CLICKABLE);
    ctx->img_down = img1;

    lv_obj_t* img2 = lv_img_create(par);
    lv_img_set_src(img2, resource_get_img("arrow_up"));
    lv_obj_align(img2, LV_ALIGN_TOP_MID, 0, PAGE_VER_RES + 12);
    lv_obj_add_event(img2, img_arrow_event_handler, LV_EVENT_ALL, ctx);
    lv_obj_add_flag(img2, LV_OBJ_FLAG_CLICKABLE);
    ctx->img_up = img2;
}

static void color_bar_create(lv_obj_t* par, color_bar_t* color_bar, int len)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    for (int i = 0; i < len; i++) {
        lv_obj_t* bar = lv_bar_create(par);
        lv_bar_set_value(bar, color_bar[i].value, LV_ANIM_OFF);
        lv_obj_set_size(bar, 156, 10);
        lv_obj_set_style_bg_color(
            bar,
            lv_color_hex(color_bar[i].color),
            LV_PART_INDICATOR);
        lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, PAGE_VER_RES + color_bar[i].y);

        lv_obj_t* label = lv_label_create(par);
        lv_obj_add_style(label, &ctx->color_bar_style, LV_PART_MAIN);
        lv_label_set_text_fmt(label, "%s %d%%", color_bar[i].text, color_bar[i].value);
        lv_obj_align_to(label, bar, LV_ALIGN_OUT_TOP_LEFT, 0, -2);
    }
}

static void chart_hr_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* chart = lv_chart_create(par);
    lv_obj_set_size(chart, 156, 87);
    lv_obj_clear_flag(chart, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(chart, LV_ALIGN_TOP_MID, 0, PAGE_VER_RES + 36);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);

    lv_obj_set_style_bg_color(
        chart,
        lv_color_make(0x1A, 0x1A, 0x1A),
        LV_PART_MAIN);
    lv_obj_set_style_border_width(chart, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(chart, 5, LV_PART_MAIN);
    lv_obj_set_style_border_color(
        chart,
        lv_color_make(0x66, 0x66, 0x66),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(chart, LV_OPA_50, LV_PART_ITEMS);
    lv_obj_set_style_bg_grad_dir(chart, LV_GRAD_DIR_VER, LV_PART_ITEMS);
    lv_obj_set_style_bg_main_stop(chart, 255, LV_PART_ITEMS);
    lv_obj_set_style_bg_grad_stop(chart, 0, LV_PART_ITEMS);
    lv_obj_set_style_line_width(chart, 1, LV_PART_ITEMS);

    lv_chart_set_div_line_count(chart, 0, 4);
    lv_obj_set_style_line_color(
        chart,
        lv_color_make(0x66, 0x66, 0x66),
        LV_PART_ITEMS);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 50, 180);
    lv_chart_set_point_count(chart, 100);

    ctx->chart_hr_ser1 = lv_chart_add_series(chart, lv_color_make(0xfc, 0x5c, 0x00), 0);

    ctx->chart_hr = chart;
}

static void auto_event_create(page_ctx_t* ctx)
{
    lv_auto_event_data_t ae_grp[EVENT_CNT] = {
        { &ctx->btn_rst, LV_EVENT_CLICKED, 2000 },
        { &ctx->img_down, LV_EVENT_CLICKED, 1000 },
        { &ctx->img_up, LV_EVENT_CLICKED, 1000 },
        { &ctx->base.obj, LV_EVENT_LEAVE, 1000 },
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
        if (dir == LV_DIR_RIGHT) {
            lv_obj_send_event(root, LV_EVENT_LEAVE, NULL);
        } else if (dir == LV_DIR_BOTTOM) {
            CONT_HR_MOVE_DOWN(false, ctx->cont_hr, ctx->img_heart);
        } else if (dir == LV_DIR_TOP) {
            CONT_HR_MOVE_DOWN(true, ctx->cont_hr, ctx->img_heart);
        }
    } else if (code == LV_EVENT_LEAVE) {
        page_pop(&ctx->base);
    }
}

static void on_page_construct(lv_fragment_t* self, void* args)
{
    LV_LOG_INFO("self: %p args: %p", self, args);

    page_ctx_t* ctx = (page_ctx_t*)self;

    color_bar_t color_bar_grp[COLOR_BAR_CNT] = {
        { "Relaxation", 45, 0x3FA9F5, 158 },
        { "Sports", 30, 0x7AC943, 212 },
        { "Competition", 20, 0xF15A24, 266 },
        { "Endurance", 5, 0xC1272D, 320 },
    };

    for (int i = 0; i < COLOR_BAR_CNT; i++) {
        ctx->color_bar_grp[i] = color_bar_grp[i];
    }
    lv_style_init(&ctx->color_bar_style);
    lv_style_set_text_font(&ctx->color_bar_style, resource_get_font(BANDX_REGULAR_FONT "_20"));
    lv_style_set_text_color(&ctx->color_bar_style, lv_color_white());

    lv_style_init(&ctx->label_hr_style1);
    lv_style_set_text_font(&ctx->label_hr_style1, resource_get_font(BANDX_REGULAR_FONT "_20"));
    lv_style_set_text_color(&ctx->label_hr_style1, lv_palette_main(LV_PALETTE_GREY));

    lv_style_init(&ctx->label_hr_style2);
    lv_style_set_text_font(&ctx->label_hr_style2, resource_get_font(BANDX_REGULAR_FONT "_20"));
    lv_style_set_text_color(&ctx->label_hr_style2, lv_color_make(0xF1, 0x5A, 0x24));
}

static void on_page_destruct(lv_fragment_t* self)
{
    LV_LOG_INFO("self: %p", self);

    page_ctx_t* ctx = (page_ctx_t*)self;

    lv_style_reset(&ctx->color_bar_style);
    lv_style_reset(&ctx->label_hr_style1);
    lv_style_reset(&ctx->label_hr_style2);
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
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(root, self);
    return root;
}

static void on_page_created(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);

    page_ctx_t* ctx = (page_ctx_t*)self;

    cont_hr_create(obj);
    label_hr_current_create(ctx->cont_hr);
    label_hr_record_create(ctx->cont_hr);
    label_hr_record_time_update(ctx->label_hr_record_time);
    btn_rst_create(ctx->cont_hr);
    img_arrow_create(ctx->cont_hr);
    chart_hr_create(ctx->cont_hr);
    color_bar_create(ctx->cont_hr, ctx->color_bar_grp, COLOR_BAR_CNT);

    ctx->timer_label_hr_update = lv_timer_create(label_hr_current_update, 1000, ctx);

    img_heart_anim_enable(ctx->img_heart, true);
    auto_event_create(ctx);
}

static void on_page_will_delete(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);

    page_ctx_t* ctx = (page_ctx_t*)self;

    lv_timer_del(ctx->timer_label_hr_update);
    img_heart_anim_enable(ctx->img_heart, false);
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

PAGE_CLASS_DEF(heart_rate);
