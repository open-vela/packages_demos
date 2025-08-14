/**
 * @file settings.c
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

#define BAR_SETTING_CNT 2
#define EVENT_CNT 9

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    const char* text;
    const char* img_src_left;
    const char* img_src_right;
    lv_coord_t y;

    lv_obj_t* img_left;
    lv_obj_t* img_right;
    lv_obj_t* bar;
} bar_setting_t;

typedef struct {
    lv_fragment_t base;
    bar_setting_t bar_setting_grp[BAR_SETTING_CNT];
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

#define BX_NAME "BandX"
#define BX_VERSION "v1.0"

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void bar_setting_event_handler(lv_event_t* event)
{
    lv_obj_t* obj = lv_event_get_user_data(event);
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_CLICKED) {
        bar_setting_t* bar_setting = (bar_setting_t*)lv_obj_get_user_data(obj);
        const char* img_src = lv_img_get_src(obj);
        int add_value = (lv_strcmp(img_src, resource_get_img(bar_setting->img_src_left)) == 0) ? -10 : +10;
        int16_t old_value = lv_bar_get_value(bar_setting->bar);
        lv_bar_set_value(bar_setting->bar, old_value + add_value, LV_ANIM_ON);
    }
}

static void img_setting_set_style(lv_obj_t* img)
{
    lv_obj_add_event(img, bar_setting_event_handler, LV_EVENT_ALL, img);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_image_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_image_recolor(img, lv_color_make(0x66, 0x66, 0x66), LV_PART_MAIN);
    lv_obj_set_style_image_recolor(img, lv_color_make(0x00, 0x89, 0xFF), LV_STATE_PRESSED);
}

static void bar_setting_create(lv_obj_t* par, bar_setting_t* bar_setting, int len)
{
    for (int i = 0; i < len; i++) {
        lv_obj_t* obj_base = lv_obj_create(par);
        lv_obj_remove_style_all(obj_base);
        lv_obj_set_size(obj_base, PAGE_HOR_RES - 40, 50);
        lv_obj_clear_flag(obj_base, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(obj_base, LV_ALIGN_TOP_MID, 0, bar_setting[i].y);

        lv_obj_t* label = lv_label_create(obj_base);
        lv_obj_set_style_text_font(label, resource_get_font(BANDX_REGULAR_FONT "_20"), LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_make(0x66, 0x66, 0x66), LV_PART_MAIN);
        lv_label_set_text(label, bar_setting[i].text);
        lv_obj_align_to(label, obj_base, LV_ALIGN_TOP_LEFT, 0, 0);

        lv_obj_t* bar = lv_bar_create(obj_base);
        lv_obj_set_size(bar, 74, 6);
        lv_obj_set_style_bg_color(bar, lv_color_make(0x00, 0x89, 0xFF), LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(bar, lv_color_make(0x33, 0x33, 0x33), LV_PART_MAIN);
        lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -10);
        lv_bar_set_value(bar, 50, LV_ANIM_OFF);
        bar_setting[i].bar = bar;

        lv_obj_t* img1 = lv_img_create(obj_base);
        lv_img_set_src(img1, resource_get_img(bar_setting[i].img_src_left));
        lv_obj_set_user_data(img1, &(bar_setting[i]));
        img_setting_set_style(img1);
        lv_obj_align_to(img1, bar, LV_ALIGN_OUT_LEFT_MID, -10, 0);

        lv_obj_t* img2 = lv_img_create(obj_base);
        lv_img_set_src(img2, resource_get_img(bar_setting[i].img_src_right));
        lv_obj_set_user_data(img2, &(bar_setting[i]));
        img_setting_set_style(img2);
        lv_obj_align_to(img2, bar, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

        bar_setting[i].img_left = img1;
        bar_setting[i].img_right = img2;
    }
}

static void sw_auto_show_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    page_ctx_t* ctx = lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        page_set_autoshow_enable(!page_get_autoshow_enable());
        if (page_get_autoshow_enable()) {
            LV_LOG_USER("Auto-show Begin ...");
            lv_obj_send_event(ctx->base.obj, LV_EVENT_LEAVE, NULL);
        } else {
            lv_auto_event_del(ctx->auto_event);
            ctx->auto_event = NULL;
            LV_LOG_USER("Auto-show End  ...");
        }
    }
}

static void sw_auto_show_create(page_ctx_t* ctx)
{
    lv_obj_t* par = ctx->base.obj;
    lv_obj_t* label = lv_label_create(par);
    lv_obj_set_style_text_font(label, resource_get_font(BANDX_REGULAR_FONT "_20"), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_make(0x66, 0x66, 0x66), LV_PART_MAIN);
    lv_label_set_text(label, "Auto-show");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 23, 187);

    lv_obj_t* sw = lv_switch_create(par);
    lv_obj_set_size(sw, 41, 23);
    lv_obj_align_to(sw, label, LV_ALIGN_OUT_RIGHT_MID, 12, -5);
    lv_obj_set_style_bg_color(sw, lv_color_make(0x33, 0x33, 0x33), LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw, lv_color_make(0x00, 0x89, 0xFF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(sw, lv_color_make(0xAA, 0xAA, 0xAA), LV_PART_KNOB);
    lv_obj_set_style_outline_width(sw, 0, LV_PART_MAIN);
    lv_obj_add_event(sw, sw_auto_show_event_handler, LV_EVENT_ALL, ctx);
    page_get_autoshow_enable() ? lv_obj_add_state(sw, LV_STATE_CHECKED) : lv_obj_clear_state(sw, LV_STATE_CHECKED);
}

static void label_info_create(lv_obj_t* par)
{
    lv_obj_t* label = lv_label_create(par);
    lv_obj_set_style_text_font(label, resource_get_font(BANDX_REGULAR_FONT "_20"), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(label, BX_NAME " " BX_VERSION "\n"__DATE__
                                     "\nBuild");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 270);
}

static void auto_event_create(page_ctx_t* ctx)
{
    bar_setting_t* bar_setting_grp = ctx->bar_setting_grp;
    lv_auto_event_data_t ae_grp[EVENT_CNT] = {
        { &(bar_setting_grp[0].img_left), LV_EVENT_CLICKED, 500 },
        { &(bar_setting_grp[0].img_left), LV_EVENT_CLICKED, 500 },
        { &(bar_setting_grp[0].img_right), LV_EVENT_CLICKED, 500 },
        { &(bar_setting_grp[0].img_right), LV_EVENT_CLICKED, 500 },
        { &(bar_setting_grp[1].img_left), LV_EVENT_CLICKED, 500 },
        { &(bar_setting_grp[1].img_left), LV_EVENT_CLICKED, 500 },
        { &(bar_setting_grp[1].img_right), LV_EVENT_CLICKED, 500 },
        { &(bar_setting_grp[1].img_right), LV_EVENT_CLICKED, 500 },
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
        }
    } else if (code == LV_EVENT_LEAVE) {
        page_pop(&ctx->base);
    }
}

static void on_page_construct(lv_fragment_t* self, void* args)
{
    LV_LOG_INFO("self: %p args: %p", self, args);

    page_ctx_t* ctx = (page_ctx_t*)self;

    bar_setting_t bar_setting_grp[BAR_SETTING_CNT] = {
        { .text = "Volume", .img_src_left = "icon_volume_reduce", .img_src_right = "icon_volume_add", 24 },
        { .text = "Backlight", .img_src_left = "icon_minus", .img_src_right = "icon_plus", 98 },
    };

    for (int i = 0; i < BAR_SETTING_CNT; i++) {
        ctx->bar_setting_grp[i] = bar_setting_grp[i];
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

    bar_setting_create(obj, ctx->bar_setting_grp, BAR_SETTING_CNT);
    sw_auto_show_create(ctx);
    label_info_create(obj);
    auto_event_create(ctx);
}

static void on_page_will_delete(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_INFO("self: %p obj: %p", self, obj);
    page_ctx_t* ctx = (page_ctx_t*)self;
    lv_auto_event_del(ctx->auto_event);
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

PAGE_CLASS_DEF(settings);
