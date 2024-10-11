/**
 * @file flashlight.c
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

#define EVENT_CNT 3

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_fragment_t base;
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

static void auto_event_create(page_ctx_t* ctx)
{
    lv_auto_event_data_t ae_grp[EVENT_CNT] = {
        { &ctx->base.obj, LV_EVENT_REFRESH, 1000 },
        { &ctx->base.obj, LV_EVENT_REFRESH, 1000 },
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

    switch (code) {
    case LV_EVENT_GESTURE: {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_RIGHT) {
            lv_obj_send_event(root, LV_EVENT_LEAVE, NULL);
        }
    } break;
    case LV_EVENT_REFRESH: {
        if (lv_obj_has_state(root, LV_STATE_CHECKED)) {
            lv_obj_remove_state(root, LV_STATE_CHECKED);
        } else {
            lv_obj_add_state(root, LV_STATE_CHECKED);
        }
    } break;
    case LV_EVENT_LEAVE: {
        page_pop(&ctx->base);
    } break;
    default:
        break;
    }
}

static void on_page_construct(lv_fragment_t* self, void* args)
{
    LV_LOG_INFO("self: %p args: %p", self, args);
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

    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);

    static const lv_style_prop_t props[] = { LV_STYLE_BG_COLOR, LV_STYLE_PROP_INV };
    static lv_style_transition_dsc_t dsc;
    lv_style_transition_dsc_init(&dsc, props, lv_anim_path_ease_out, 200, 0, NULL);

    lv_obj_set_style_bg_color(obj, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, lv_color_mix(lv_color_white(), lv_color_black(), 200), LV_STATE_CHECKED);
    lv_obj_set_style_transition(obj, &dsc, LV_STATE_DEFAULT);
    lv_obj_set_style_transition(obj, &dsc, LV_STATE_CHECKED);
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

PAGE_CLASS_DEF(flashlight);
