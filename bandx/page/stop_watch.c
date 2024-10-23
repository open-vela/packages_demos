/**
 * @file stop_watch.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "page.h"

#include "../utils/lv_obj_ext_func.h"

/*********************
 *      DEFINES
 *********************/

#define BTN_WIDTH 83
#define BTN_HEIGHT 60
#define BTN_CNT 2
#define EVENT_CNT 10

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    const char* img_src;
    lv_obj_t* btn;
    lv_obj_t* img;
} btn_sw_t;

typedef struct {
    lv_obj_t* list_btn;
    const char* text;
    const uint16_t btn_val;
} list_btn_t;

enum BtnGrpEnum {
    BTN_START_PAUSE,
    BTN_FLAG_RESET,
};

typedef struct {
    lv_fragment_t base;
    lv_obj_t* label_time;
    lv_obj_t* list_history;
    lv_timer_t* timer_sw_update;
    btn_sw_t btn_grp[BTN_CNT];

    lv_coord_t label_time_record_y;
    lv_coord_t label_time_ready_y;

    bool sw_is_pause;
    uint32_t sw_current_time;
    uint32_t sw_interval_time;
    uint32_t sw_last_time;
    uint8_t sw_history_record_cnt;
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

static void line_sw_create(lv_obj_t* par)
{
    static const lv_point_precise_t line_points[] = { { 0, 0 }, { 170, 0 } };

    lv_obj_t* line = lv_line_create(par);
    lv_line_set_points(line, line_points, ARRAY_SIZE(line_points));

    lv_obj_set_style_line_color(
        line,
        lv_color_make(0x80, 0x80, 0x80),
        LV_STATE_DEFAULT);
    lv_obj_set_style_line_width(line, 2, LV_STATE_DEFAULT);
    lv_obj_align_to(line, par, LV_ALIGN_TOP_MID, 0, 286);
}

static void list_history_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* list = lv_list_create(par);
    lv_obj_set_style_text_font(list, resource_get_font(BANDX_REGULAR_FONT "_20"), LV_PART_MAIN);
    lv_obj_set_style_bg_color(list, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);

    lv_obj_set_size(list, 170, 170);
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 0);
    ctx->list_history = list;
}

static void label_time_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* label = lv_label_create(par);
    lv_obj_set_style_text_font(label, resource_get_font(BANDX_REGULAR_FONT "_38"), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0x87FFCE), LV_PART_MAIN);

    lv_label_set_text(label, "00:00.000");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, ctx->label_time_ready_y);
    lv_obj_set_width(label, 180);
    lv_obj_set_style_border_color(label, lv_color_make(0x80, 0x80, 0x80), LV_PART_MAIN);
    lv_obj_set_style_border_width(label, 2, LV_PART_MAIN);
    lv_obj_set_style_border_side(label, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    ctx->label_time = label;
}

static void sw_show_time(uint32_t ms, lv_obj_t* label_time)
{
    uint16_t min = 0, sec = 0, msec = 0;
    min = ms / 60000;
    sec = (ms - (min * 60000)) / 1000;
    msec = ms - (sec * 1000) - (min * 60000);
    lv_label_set_text_fmt(label_time, "%02d:%02d.%03d", min, sec, msec);
}

static void sw_pause(page_ctx_t* ctx)
{
    if (ctx->sw_is_pause) {
        ctx->sw_interval_time = lv_tick_get() - ctx->sw_last_time;
        lv_timer_resume(ctx->timer_sw_update);
        ctx->sw_is_pause = false;
    } else {
        ctx->sw_last_time = ctx->sw_current_time;
        lv_timer_pause(ctx->timer_sw_update);
        ctx->sw_is_pause = true;
    }
}

static void sw_reset(page_ctx_t* ctx)
{
    ctx->sw_last_time = 0;
    ctx->sw_current_time = 0;
    ctx->sw_interval_time = 0;
    ctx->sw_is_pause = true;
    ctx->sw_history_record_cnt = 0;
    lv_timer_pause(ctx->timer_sw_update);
    sw_show_time(ctx->sw_current_time, ctx->label_time);
    lv_obj_clean(ctx->list_history);

    if (lv_obj_get_y(ctx->label_time) == ctx->label_time_record_y) {
        LV_OBJ_ADD_ANIM(ctx->label_time, y, ctx->label_time_ready_y, 500);
    }
}

static void sw_update(lv_timer_t* timer)
{
    page_ctx_t* ctx = lv_timer_get_user_data(timer);

    ctx->sw_current_time = lv_tick_get() - ctx->sw_interval_time;
    sw_show_time(ctx->sw_current_time, ctx->label_time);
}

static void sw_record(page_ctx_t* ctx)
{
    lv_obj_t* list_btn;
    lv_obj_t* spangroup;
    uint16_t min = 0, sec = 0, msec = 0;

    if (ctx->sw_history_record_cnt < 10 && !ctx->sw_is_pause) {
        min = ctx->sw_current_time / 60000;
        sec = (ctx->sw_current_time - (min * 60000)) / 1000;
        msec = ctx->sw_current_time - (sec * 1000) - (min * 60000);

        char cnt_buf[8];
        char time_buf[64];

        lv_snprintf(cnt_buf, sizeof(cnt_buf), "%d. ", ctx->sw_history_record_cnt + 1);
        lv_snprintf(time_buf, sizeof(time_buf), "%02d:%02d.%03d", min, sec, msec);

        list_btn = lv_list_add_btn(ctx->list_history, NULL, NULL);
        lv_obj_clear_flag(list_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_border_width(list_btn, 2, LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(list_btn, 0, LV_PART_MAIN);

        spangroup = lv_spangroup_create(list_btn);

        lv_span_t* cnt_span = lv_spangroup_new_span(spangroup);
        lv_span_set_text(cnt_span, cnt_buf);
        lv_style_set_text_color(&cnt_span->style, lv_color_hex(0x87FFCE));

        lv_span_t* time_span = lv_spangroup_new_span(spangroup);
        lv_span_set_text(time_span, time_buf);
        lv_style_set_text_color(&time_span->style, lv_color_hex(0xffffff));

        lv_obj_add_state(list_btn, LV_STATE_FOCUSED);
        lv_obj_set_style_bg_color(list_btn, lv_color_black(), 0);
        lv_obj_set_style_text_decor(
            list_btn,
            LV_TEXT_DECOR_NONE,
            LV_STATE_FOCUSED);
        ctx->sw_history_record_cnt++;
    }

    if (lv_obj_get_y(ctx->label_time) == ctx->label_time_ready_y) {
        LV_OBJ_ADD_ANIM(ctx->label_time, y, ctx->label_time_record_y, 500);
    }
}

static void btn_grp_event_handler(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* obj = lv_event_get_current_target_obj(event);
    page_ctx_t* ctx = (page_ctx_t*)lv_event_get_user_data(event);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* img = lv_obj_get_child(obj, 0);
        const char* img_src = lv_img_get_src(img);

        if (obj == ctx->btn_grp[BTN_START_PAUSE].btn) {
            sw_pause(ctx);
            if (ctx->sw_is_pause) {
                lv_img_set_src(ctx->btn_grp[BTN_START_PAUSE].img, resource_get_img("icon_start"));
                lv_img_set_src(ctx->btn_grp[BTN_FLAG_RESET].img, resource_get_img("icon_reset"));
            } else {
                lv_img_set_src(ctx->btn_grp[BTN_START_PAUSE].img, resource_get_img("icon_pause"));
                lv_img_set_src(ctx->btn_grp[BTN_FLAG_RESET].img, resource_get_img("icon_flag"));
                lv_obj_clear_state(ctx->btn_grp[BTN_FLAG_RESET].btn, LV_STATE_DISABLED);
            }
        } else if (obj == ctx->btn_grp[BTN_FLAG_RESET].btn) {
            if (lv_strcmp(img_src, resource_get_img("icon_flag")) == 0) {
                sw_record(ctx);
            } else {
                if (ctx->sw_is_pause) {
                    sw_reset(ctx);
                    lv_obj_add_state(ctx->btn_grp[BTN_FLAG_RESET].btn, LV_STATE_DISABLED);
                    ctx->sw_history_record_cnt = 0;
                }
            }
        }
    }
}

static void btn_grp_create(lv_obj_t* par, btn_sw_t* btn_sw, int len)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    for (int i = 0; i < len; i++) {
        lv_obj_t* btn = lv_btn_create(par);
        lv_obj_set_size(btn, BTN_WIDTH, BTN_HEIGHT);

        lv_coord_t x_ofs = BTN_WIDTH / 2 + 3;
        lv_obj_align(btn, LV_ALIGN_TOP_MID, i == 0 ? -x_ofs : x_ofs, 300);
        lv_obj_set_style_border_color(btn, lv_color_black(), LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, 2, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(
            btn,
            lv_palette_main(LV_PALETTE_GREY),
            LV_STATE_PRESSED || LV_STATE_DISABLED);
        lv_obj_set_style_bg_color(
            btn,
            lv_color_make(0x87, 0xFF, 0xCE),
            LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);
        lv_obj_add_event(btn, btn_grp_event_handler, LV_EVENT_ALL, ctx);

        lv_obj_t* img = lv_img_create(btn);
        lv_img_set_src(img, resource_get_img(btn_sw[i].img_src));
        lv_obj_align_to(img, btn, LV_ALIGN_CENTER, 0, 0);

        btn_sw[i].btn = btn;
        btn_sw[i].img = img;
    }
}

static void auto_event_create(page_ctx_t* ctx)
{
    btn_sw_t* btn_sw = ctx->btn_grp;
    lv_auto_event_data_t ae_grp[EVENT_CNT] = {
        { &(btn_sw[BTN_START_PAUSE].btn), LV_EVENT_CLICKED, 2000 },
        { &(btn_sw[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 453 },
        { &(btn_sw[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 888 },
        { &(btn_sw[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 1605 },
        { &(btn_sw[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 785 },
        { &(btn_sw[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 1200 },
        { &(btn_sw[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 1456 },
        { &(btn_sw[BTN_START_PAUSE].btn), LV_EVENT_CLICKED, 2000 },
        { &(btn_sw[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 1000 },
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
        if (dir == LV_DIR_RIGHT) {
            lv_obj_send_event(root, LV_EVENT_LEAVE, NULL);
        }
    } else if (code == LV_EVENT_LEAVE) {
        page_pop(&ctx->base);
    }
}

static void on_page_construct(lv_fragment_t* self, void* args)
{
    LV_LOG_INFO("self: %p args: %p", self, args);

    page_ctx_t* ctx = (page_ctx_t*)self;

    btn_sw_t btn_grp[BTN_CNT] = {
        { "icon_start" },
        { "icon_reset" },
    };

    for (int i = 0; i < BTN_CNT; i++) {
        ctx->btn_grp[i] = btn_grp[i];
    }

    ctx->label_time_record_y = 18;
    ctx->label_time_ready_y = 64;

    ctx->sw_is_pause = true;
    ctx->sw_current_time = 0;
    ctx->sw_interval_time = 0;
    ctx->sw_last_time = 0;
    ctx->sw_history_record_cnt = 0;
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
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(root, self);
    return root;
}

static void on_page_created(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);

    page_ctx_t* ctx = (page_ctx_t*)self;

    ctx->timer_sw_update = lv_timer_create(sw_update, 51, ctx);

    list_history_create(obj);
    label_time_create(obj);
    btn_grp_create(obj, ctx->btn_grp, BTN_CNT);
    line_sw_create(obj);
    auto_event_create(ctx);

    ctx->sw_is_pause ? lv_timer_pause(ctx->timer_sw_update) : lv_timer_resume(ctx->timer_sw_update);
}

static void on_page_will_delete(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);

    page_ctx_t* ctx = (page_ctx_t*)self;

    lv_timer_del(ctx->timer_sw_update);
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

PAGE_CLASS_DEF(stop_watch);
