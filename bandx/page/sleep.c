/**
 * @file sleep.c
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

#define SLEEP_TIME_CNT 6
#define SLEEP_INFO_CNT 4
#define EVENT_CNT 3

/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    SLEEP_TYPE_SHALLOW,
    SLEEP_TYPE_DEEP,
    SLEEP_TYPE_REM,
    SLEEP_TYPE_AWAKE,
    SLEEP_TYPE_MAX
} sleep_type;

typedef struct {
    const char* text;
    sleep_type type;
} sleep_info_t;

typedef struct {
    sleep_type type;
    uint32_t start_min;
    uint32_t end_min;
} sleep_time_t;

typedef struct {
    lv_fragment_t base;
    lv_obj_t* img_down;
    lv_obj_t* img_up;
    lv_obj_t* cont_sleep;

    uint32_t sleep_type_color[SLEEP_TYPE_MAX];
    sleep_time_t sleep_time_grp[SLEEP_TIME_CNT];
    sleep_info_t sleep_info_grp[SLEEP_INFO_CNT];
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

#define CONT_SLEEP_MOVE_DOWN(down, obj) \
    LV_OBJ_ADD_ANIM(                    \
        obj,                            \
        y,                              \
        (down) ? -PAGE_VER_RES : 0, LV_ANIM_TIME_DEFAULT)

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void cont_sleep_create(lv_obj_t* par)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);
    lv_obj_t* obj = lv_obj_create(par);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, PAGE_HOR_RES, PAGE_VER_RES * 2);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_user_data(obj, ctx);

    ctx->cont_sleep = obj;
}

static void img_arrow_event_handler(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* obj = lv_event_get_current_target_obj(event);
    page_ctx_t* ctx = (page_ctx_t*)lv_event_get_user_data(event);

    if (code == LV_EVENT_CLICKED) {
        bool is_down = (lv_strcmp(lv_img_get_src(obj), resource_get_img("arrow_down")) == 0);
        CONT_SLEEP_MOVE_DOWN(is_down, ctx->cont_sleep);
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

    lv_obj_t* img2 = lv_img_create(par);
    lv_img_set_src(img2, resource_get_img("arrow_up"));
    lv_obj_align(img2, LV_ALIGN_TOP_MID, 0, PAGE_VER_RES + 12);
    lv_obj_add_event(img2, img_arrow_event_handler, LV_EVENT_ALL, ctx);
    lv_obj_add_flag(img2, LV_OBJ_FLAG_CLICKABLE);

    ctx->img_down = img1;
    ctx->img_up = img2;
}

static void img_sleep_create(lv_obj_t* par)
{
    lv_obj_t* img = lv_img_create(par);
    lv_img_set_src(img, resource_get_img("sleep"));
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 32);
}

static void label_score_create(lv_obj_t* par)
{
    lv_obj_t* label1 = lv_label_create(par);
    lv_obj_set_style_text_font(
        label1,
        resource_get_font(BANDX_REGULAR_FONT "_20"),
        LV_PART_MAIN);
    lv_obj_set_style_text_color(label1, lv_color_hex(0x808080), LV_PART_MAIN);
    lv_label_set_text(label1, "Score");
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 123);

    lv_obj_t* label2 = lv_label_create(par);
    lv_obj_set_style_text_font(
        label2,
        resource_get_font(BANDX_REGULAR_FONT "_72"),
        LV_PART_MAIN);
    lv_obj_set_style_text_color(label2, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(label2, "78");
    lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 148);
}

static void label_total_time_create(lv_obj_t* par)
{
    lv_obj_t* label1 = lv_label_create(par);
    lv_obj_set_style_text_font(label1, resource_get_font(BANDX_REGULAR_FONT "_20"), LV_PART_MAIN);
    lv_obj_set_style_text_color(label1, lv_color_hex(0x808080), LV_PART_MAIN);
    lv_label_set_text(label1, "Total time");
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 224);

    lv_obj_t* label2 = lv_label_create(par);
    lv_obj_set_style_text_font(label2, resource_get_font(BANDX_REGULAR_FONT "_20"), LV_PART_MAIN);
    lv_obj_set_style_text_color(label2, lv_color_hex(0x3FA9F5), LV_PART_MAIN);
    lv_label_set_text(label2, "8 h 52 min");
    lv_obj_set_style_text_align(label2, LV_ALIGN_CENTER, 0);
    lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 249);

    lv_obj_t* spangroup = lv_spangroup_create(par);
    lv_obj_set_style_text_font(spangroup, resource_get_font(BANDX_REGULAR_FONT "_20"), LV_PART_MAIN);

    lv_span_t* span = lv_spangroup_new_span(spangroup);
    lv_span_set_text_static(span, "00:29");
    lv_style_set_text_color(&span->style, lv_color_hex(0x3FA9F5));

    lv_span_t* span1 = lv_spangroup_new_span(spangroup);
    lv_span_set_text_static(span1, "-");
    lv_style_set_text_color(&span1->style, lv_color_hex(0x808080));

    lv_span_t* span2 = lv_spangroup_new_span(spangroup);
    lv_span_set_text_static(span2, "09:48");
    lv_style_set_text_color(&span2->style, lv_color_hex(0x3FA9F5));
    lv_obj_align(spangroup, LV_ALIGN_TOP_MID, 0, 274);
}

static uint32_t sleep_time_get_sum(sleep_time_t* sleep_time, int len, sleep_type check_type)
{
    uint32_t sum = 0;
    if (check_type == SLEEP_TYPE_SHALLOW) {
        sum = sleep_time[0].end_min - sleep_time[0].start_min;
        sum -= sleep_time_get_sum(sleep_time, len, SLEEP_TYPE_DEEP);
        sum -= sleep_time_get_sum(sleep_time, len, SLEEP_TYPE_REM);
        sum -= sleep_time_get_sum(sleep_time, len, SLEEP_TYPE_AWAKE);
    } else {
        for (int i = 0; i < len; i++) {
            if (sleep_time[i].type == check_type) {
                uint32_t time = sleep_time[i].end_min - sleep_time[i].start_min;
                sum += time;
            }
        }
    }
    return sum;
}

static void obj_sleep_time_create(lv_obj_t* par, sleep_time_t* sleep_time, int len)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    lv_obj_t* obj_base = lv_obj_create(par);
    lv_obj_remove_style_all(obj_base);
    lv_obj_set_size(obj_base, 165, 74);
    lv_obj_set_style_radius(obj_base, 10, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(obj_base, 10, LV_PART_MAIN);

    lv_obj_align(obj_base, LV_ALIGN_TOP_MID, 0, PAGE_VER_RES + 42);
    lv_obj_clear_flag(obj_base, LV_OBJ_FLAG_SCROLLABLE);

    uint32_t time_len = sleep_time[0].end_min - sleep_time[0].start_min;
    uint32_t time_start = sleep_time[0].start_min;
    lv_coord_t width_base = lv_obj_get_style_width(obj_base, 0);

    for (int i = 0; i < len; i++) {
        uint8_t type_index = sleep_time[i].type;
        lv_color_t color = lv_color_hex(ctx->sleep_type_color[type_index]);

        lv_obj_t* obj = lv_obj_create(obj_base);
        lv_obj_set_style_radius(obj, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(obj, color, LV_PART_MAIN);

        lv_coord_t width = width_base * (sleep_time[i].end_min - sleep_time[i].start_min) / time_len;
        lv_coord_t x_ofs = width_base * (sleep_time[i].start_min - time_start) / time_len;

        lv_obj_set_size(obj, width, lv_obj_get_style_height(obj_base, 0));
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(obj, LV_ALIGN_TOP_LEFT, x_ofs, 0);
    }

    lv_obj_t* label1 = lv_label_create(par);
    lv_obj_set_style_text_font(
        label1,
        resource_get_font(BANDX_REGULAR_FONT "_15"),
        LV_PART_MAIN);
    lv_obj_set_style_text_color(
        label1,
        lv_color_make(0x80, 0x80, 0x80),
        LV_PART_MAIN);

    uint32_t start_min = sleep_time[0].start_min;
    lv_label_set_text_fmt(label1, "%02" LV_PRIu32 ":%02" LV_PRIu32, start_min / 60, start_min % 60);
    lv_obj_align_to(label1, obj_base, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t* label2 = lv_label_create(par);
    lv_obj_set_style_text_font(
        label2,
        resource_get_font(BANDX_REGULAR_FONT "_15"),
        LV_PART_MAIN);
    lv_obj_set_style_text_color(
        label2,
        lv_color_make(0x80, 0x80, 0x80),
        LV_PART_MAIN);

    uint32_t end_min = sleep_time[0].end_min;
    lv_label_set_text_fmt(label2, "%02" LV_PRIu32 ":%02" LV_PRIu32, end_min / 60, end_min % 60);
    lv_obj_align_to(label2, obj_base, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 5);
}

static void obj_sleep_info_create(lv_obj_t* par, sleep_info_t* sleep_info, int len)
{
    page_ctx_t* ctx = (page_ctx_t*)lv_obj_get_user_data(par);

    for (int i = 0; i < len; i++) {
        lv_obj_t* obj_base = lv_obj_create(par);
        lv_obj_remove_style_all(obj_base);
        lv_obj_set_style_bg_opa(obj_base, LV_OPA_0, LV_PART_MAIN);
        lv_obj_set_size(obj_base, PAGE_HOR_RES - 40, 50);
        lv_obj_clear_flag(obj_base, LV_OBJ_FLAG_SCROLLABLE);

        lv_coord_t y_ofs = PAGE_VER_RES + 150 + (lv_obj_get_style_height(obj_base, 0) + 5) * i;
        lv_obj_align(obj_base, LV_ALIGN_TOP_MID, 0, y_ofs);

        lv_obj_t* label1 = lv_label_create(obj_base);
        lv_obj_set_style_text_font(
            label1,
            resource_get_font(BANDX_REGULAR_FONT "_20"),
            LV_PART_MAIN);
        lv_obj_set_style_text_color(
            label1,
            lv_color_make(0x80, 0x80, 0x80),
            LV_PART_MAIN);
        lv_label_set_text(label1, sleep_info[i].text);
        lv_obj_align_to(label1, obj_base, LV_ALIGN_TOP_LEFT, 0, 0);

        lv_obj_t* label2 = lv_label_create(obj_base);
        lv_obj_set_style_text_font(
            label2,
            resource_get_font(BANDX_REGULAR_FONT "_20"),
            LV_PART_MAIN);
        lv_obj_set_style_text_color(
            label2,
            lv_color_white(),
            LV_PART_MAIN);
        uint32_t time = sleep_time_get_sum(
            ctx->sleep_time_grp,
            SLEEP_TIME_CNT,
            sleep_info[i].type);
        lv_label_set_text_fmt(label2, "%" LV_PRIu32 "h %" LV_PRIu32 "min", time / 60, time % 60);
        lv_obj_align_to(label2, obj_base, LV_ALIGN_BOTTOM_LEFT, 0, 0);

        lv_obj_t* label3 = lv_label_create(obj_base);
        lv_obj_set_style_text_font(
            label3,
            resource_get_font(BANDX_REGULAR_FONT "_20"),
            LV_PART_MAIN);
        lv_obj_set_style_text_color(
            label3,
            lv_color_hex(ctx->sleep_type_color[sleep_info[i].type]),
            LV_PART_MAIN);
        uint32_t time_len = ctx->sleep_time_grp[0].end_min - ctx->sleep_time_grp[0].start_min;
        lv_label_set_text_fmt(label3, "%" LV_PRIu32 "%%", time * 100 / time_len);
        lv_obj_align_to(label3, obj_base, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    }
}

static void auto_event_create(page_ctx_t* ctx)
{
    lv_auto_event_data_t ae_grp[EVENT_CNT] = {
        { &ctx->img_down, LV_EVENT_CLICKED, 2000 },
        { &ctx->img_up, LV_EVENT_CLICKED, 2000 },
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
        } else if (dir == LV_DIR_BOTTOM) {
            CONT_SLEEP_MOVE_DOWN(false, ctx->cont_sleep);
        } else if (dir == LV_DIR_TOP) {
            CONT_SLEEP_MOVE_DOWN(true, ctx->cont_sleep);
        }
    } else if (code == LV_EVENT_LEAVE) {
        page_pop(&ctx->base);
    }
}

static void on_page_construct(lv_fragment_t* self, void* args)
{
    LV_LOG_INFO("self: %p args: %p", self, args);

    page_ctx_t* ctx = (page_ctx_t*)self;

    uint32_t sleep_type_color[SLEEP_TYPE_MAX] = {
        0x3f71f5,
        0x023192,
        0x7ac943,
        0xff931e,
    };

    for (int i = 0; i < SLEEP_TYPE_MAX; i++) {
        ctx->sleep_type_color[i] = sleep_type_color[i];
    }

    sleep_time_t sleep_time_grp[SLEEP_TIME_CNT] = {
        { SLEEP_TYPE_SHALLOW, 29, 588 },
        { SLEEP_TYPE_DEEP, 61, 80 },
        { SLEEP_TYPE_DEEP, 120, 200 },
        { SLEEP_TYPE_REM, 80, 95 },
        { SLEEP_TYPE_REM, 250, 310 },
        { SLEEP_TYPE_AWAKE, 400, 410 },
    };

    for (int i = 0; i < SLEEP_TIME_CNT; i++) {
        ctx->sleep_time_grp[i] = sleep_time_grp[i];
    }

    sleep_info_t sleep_info_grp[SLEEP_INFO_CNT] = {
        { "Shallow sleep", SLEEP_TYPE_SHALLOW },
        { "Deep sleep", SLEEP_TYPE_DEEP },
        { "REM", SLEEP_TYPE_REM },
        { "Awake", SLEEP_TYPE_AWAKE },
    };

    for (int i = 0; i < SLEEP_INFO_CNT; i++) {
        ctx->sleep_info_grp[i] = sleep_info_grp[i];
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
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(root, self);
    return root;
}

static void on_page_created(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);

    page_ctx_t* ctx = (page_ctx_t*)self;

    cont_sleep_create(obj);

    img_arrow_create(ctx->cont_sleep);
    img_sleep_create(ctx->cont_sleep);
    label_score_create(ctx->cont_sleep);
    label_total_time_create(ctx->cont_sleep);

    obj_sleep_time_create(ctx->cont_sleep, ctx->sleep_time_grp, SLEEP_TIME_CNT);
    obj_sleep_info_create(ctx->cont_sleep, ctx->sleep_info_grp, SLEEP_INFO_CNT);
    auto_event_create(ctx);
}

static void on_page_will_delete(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);

    page_ctx_t* ctx = (page_ctx_t*)self;
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

PAGE_CLASS_DEF(sleep);
