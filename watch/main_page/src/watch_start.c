#include "watch_start.h"
#include "main_page.h"



void dele_watch_start(lv_obj_t* obj)
{
    lv_obj_delete(obj);
}

static void event_cb(lv_event_t * e)
{
    LV_LOG_USER("event_cb");
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_current_target(e);
    lv_obj_t* parent = lv_obj_get_parent(obj);
    LV_LOG_USER("obj: %p-------------------parent: %p \n",obj,parent);
    switch (code) {
        case LV_EVENT_CLICKED:
            dele_watch_start(parent);
            main_page("digital_dial");
        
            break;
        default:
            break;
    }
}

void watch_start(void)
{
    LV_LOG_USER("watch_start in");
    lv_obj_t* watchScr = create_watch_scr();

    lv_obj_t* label = lv_label_create(watchScr);
    lv_label_set_text(label, "Welcome use");

    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -100);

    // todo
    // use lv_style_set_shadow_*** to optimize
    lv_obj_t* okBtn = lv_button_create(watchScr);
    lv_obj_set_size(okBtn, 50, 50);
    lv_obj_set_style_radius(okBtn, 25, 0);
    lv_obj_set_style_border_opa(okBtn, 50, 0);
    lv_obj_align(okBtn, LV_ALIGN_CENTER, 0, 100);
    lv_obj_t* okLabel = lv_label_create(okBtn);
    lv_label_set_text(okLabel, LV_SYMBOL_OK);
    // lv_obj_set_style_text_font(okLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(okLabel);
    // 设置热区扩展（单位：像素）
    lv_obj_set_ext_click_area(okBtn, 50);
    lv_obj_add_event_cb(okBtn, event_cb, LV_EVENT_CLICKED, 0);
}