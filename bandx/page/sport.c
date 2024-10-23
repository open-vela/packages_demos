/**
 * @file sport.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "page.h"

/*********************
 *      DEFINES
 *********************/

#define BTN_WIDTH 170
#define BTN_HEIGHT 50
#define SPORT_INFO_CNT 9
#define EVENT_CNT 2

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    const char* img_src_name;
    const char* sport_name;
    uint32_t color;
} sport_info_t;

typedef struct {
    lv_fragment_t base;
    lv_obj_t* list;
    sport_info_t sport_info_grp[SPORT_INFO_CNT];
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

static void on_sport_icon_event(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_CLICKED) {
        LV_LOG_WARN("NO match for this sport.");
    }
}
static void on_sport_list_event(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* last_obj = lv_event_get_user_data(event);
    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_scroll_to_view(last_obj, LV_ANIM_ON);
    }
}

static void sport_title_create(lv_obj_t* par)
{
    lv_obj_t* obj = lv_obj_create(par);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, PAGE_HOR_RES, 35);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t* label = lv_label_create(obj);
    lv_label_set_text(label, "Sports");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, resource_get_font(BANDX_REGULAR_FONT "_15"), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

static void sport_list_create(page_ctx_t* ctx)
{
    ctx->list = lv_obj_create(ctx->base.obj);
    lv_obj_remove_style_all(ctx->list);
    lv_obj_set_size(ctx->list, PAGE_HOR_RES, PAGE_VER_RES - 40);
    lv_obj_set_flex_flow(ctx->list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ctx->list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(ctx->list, 10, 0);
    lv_obj_align(ctx->list, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t* obj_base = NULL;

    for (int i = 0; i < SPORT_INFO_CNT; i++) {

        obj_base = lv_btn_create(ctx->list);
        lv_obj_set_style_bg_color(obj_base, lv_color_hex(0x666666), LV_PART_MAIN);
        lv_obj_set_style_border_width(obj_base, 0, LV_PART_MAIN);
        lv_obj_set_size(obj_base, BTN_WIDTH, BTN_HEIGHT);
        lv_obj_set_style_pad_left(obj_base, 10, LV_PART_MAIN);
        lv_obj_add_event(obj_base, on_sport_icon_event, LV_EVENT_ALL, NULL);

        lv_obj_t* img = lv_img_create(obj_base);
        lv_img_set_src(img, resource_get_img(ctx->sport_info_grp[i].img_src_name));
        lv_obj_set_style_image_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_image_recolor(img, lv_color_hex(ctx->sport_info_grp[i].color), LV_PART_MAIN);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_t* label = lv_label_create(obj_base);
        lv_label_set_text(label, ctx->sport_info_grp[i].sport_name);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, resource_get_font(BANDX_REGULAR_FONT "_15"), LV_PART_MAIN);
        lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, 4, 0);
    }
    lv_obj_add_event(ctx->list, on_sport_list_event, LV_EVENT_ALL, obj_base);
}

static void auto_event_create(page_ctx_t* ctx)
{
    lv_auto_event_data_t ae_grp[EVENT_CNT] = {
        { &ctx->list, LV_EVENT_VALUE_CHANGED, 1000 },
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

    sport_info_t sport_info_grp[SPORT_INFO_CNT] = {
        { "icon_running", "Running", 0x9AFF9A },
        { "icon_stretching", "Stretching", 0xFFF68F },
        { "icon_riding", "Riding", 0xFFC1C1 },
        { "icon_skiing", "Skiing", 0xAB82FF },
        { "icon_yoga", "Yoga", 0x97FFFF },
        { "icon_tabletennis", "Table tennis", 0xFF83FA },
        { "icon_basketball", "Basketball", 0xFFA54F },
        { "icon_volleyball", "Volleyball", 0x87CEFA },
        { "icon_more_sports", "More sports", 0x4876FF },
    };

    for (int i = 0; i < SPORT_INFO_CNT; i++) {
        ctx->sport_info_grp[i] = sport_info_grp[i];
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

    sport_title_create(obj);
    sport_list_create(ctx);
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

PAGE_CLASS_DEF(sport);
