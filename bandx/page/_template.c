/**
 * @file _template.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "page.h"

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_fragment_t base;
    lv_obj_t* label;
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

static void on_page_construct(lv_fragment_t* self, void* args)
{
    LV_LOG_USER("self: %p args: %p", self, args);
}

static void on_page_destruct(lv_fragment_t* self)
{
    LV_LOG_USER("self: %p", self);
}

static void on_page_attached(lv_fragment_t* self)
{
    LV_LOG_USER("self: %p", self);
}

static void on_page_detached(lv_fragment_t* self)
{
    LV_LOG_USER("self: %p", self);
}

static lv_obj_t* on_page_create(lv_fragment_t* self, lv_obj_t* container)
{
    LV_LOG_USER("self: %p container: %p", self, container);

    lv_obj_t* root = lv_obj_create(container);
    lv_obj_remove_style_all(root);
    lv_obj_add_style(root, resource_get_style("root_def"), 0);
    lv_obj_set_style_bg_color(root, lv_palette_main(LV_PALETTE_BLUE), 0);
    return root;
}

static void on_page_created(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_USER("self: %p obj: %p", self, obj);

    lv_obj_t* label = lv_label_create(obj);
    lv_obj_set_style_text_font(label, resource_get_font(BANDX_REGULAR_FONT "_38"), 0);
    lv_label_set_text(label, "SPORT");
    lv_obj_center(label);

    lv_obj_t* img = lv_img_create(obj);
    lv_img_set_src(img, resource_get_img("sport"));
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 20);
}

static void on_page_will_delete(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_USER("self: %p obj: %p", self, obj);
}

static void on_page_deleted(lv_fragment_t* self, lv_obj_t* obj)
{
    LV_LOG_USER("self: %p obj: %p", self, obj);
}

static bool on_page_event(lv_fragment_t* self, int code, void* user_data)
{
    LV_LOG_USER("self: %p code: %d user_data: %p", self, code, user_data);
    return false;
}

PAGE_CLASS_DEF(template);
