
#include <string.h>
#include "choose_dial.h"

static lv_style_t circle; // 圆形的通用界面风格
static void click_event_cb(lv_event_t *e)
{
    LV_LOG_USER("click_event_cb in");
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *img = lv_event_get_current_target(e);
    lv_obj_t *parent = lv_obj_get_parent(img);
    const char *buf = lv_event_get_user_data(e);
    LV_LOG_USER("buf = %s", buf);

    switch (code)
    {
    case LV_EVENT_CLICKED:
        lv_obj_delete(parent);
        if (!strcmp(buf, "clock_dial"))
        {
            main_page("clock_dial");
        }
        if (!strcmp(buf, "digital_dial"))
        {
            main_page("digital_dial");
        }
        break;
    default:
        break;
    }
}

void choose_dial(const char *str)
{
    LV_LOG_USER("choose_dial in------");

    lv_style_init(&circle);
    lv_style_set_radius(&circle, 116); // 455的一半，保持圆形
    lv_obj_t *chooseDial = create_watch_scr();

    lv_obj_t *clockDial;
    lv_obj_t *digitalDial;
    lv_obj_t *add_screen;
    if (!strcmp(str, "clock_dial"))
    { // if user choose clock page
        clockDial = lv_obj_create(chooseDial);
        lv_obj_set_style_bg_color(clockDial, lv_color_hex(0x030301), 0);
        lv_obj_set_scrollbar_mode(clockDial, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_size(clockDial, 250, 280);
        lv_obj_add_style(clockDial,&circle,0);//new code
        lv_obj_center(clockDial);
        lv_obj_add_flag(clockDial, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(clockDial, click_event_cb, LV_EVENT_CLICKED, "clock_dial");

        // lv_obj_t* clockImg = lv_image_create(clockDial);
        // LV_IMAGE_DECLARE(clock_dial_choose);
        // lv_image_set_src(clockImg, &clock_dial_choose);
        // lv_obj_center(clockImg);

        lv_obj_t *clockLabel = lv_label_create(clockDial);
        lv_label_set_text(clockLabel, "ClockDial");
        lv_obj_set_style_text_color(clockLabel, lv_color_white(), 0);
        lv_obj_set_style_border_width(clockDial, 0, 0);
        lv_obj_align_to(clockLabel, clockDial, LV_ALIGN_CENTER, 0, 0); // new code

        digitalDial = lv_obj_create(chooseDial);
        lv_obj_set_style_bg_color(digitalDial, lv_color_hex(0x3A7BD5), 0); // new code
        lv_obj_set_scrollbar_mode(digitalDial, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_size(digitalDial, 250, 280);
         lv_obj_add_style(digitalDial,&circle,0);//new code
        lv_obj_align_to(digitalDial, clockDial, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
        lv_obj_add_flag(digitalDial, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(digitalDial, click_event_cb, LV_EVENT_CLICKED, "digital_dial");

        // lv_obj_t* digitalImg = lv_image_create(digitalDial);
        // LV_IMAGE_DECLARE(digital_dial_choose);
        // lv_image_set_src(digitalImg, &digital_dial_choose);
        // lv_obj_center(digitalImg);

        lv_obj_t *digitalLabel = lv_label_create(digitalDial);
        lv_label_set_text(digitalLabel, "DigitalDial");
        lv_obj_set_style_text_color(digitalLabel, lv_color_white(), 0);
        lv_obj_set_style_border_width(digitalDial, 0, 0);
        lv_obj_align_to(digitalLabel, digitalDial, LV_ALIGN_CENTER, 0, 0);

        add_screen = lv_obj_create(chooseDial);
        // lv_obj_add_style(add_screen, &style_face, 0);
        lv_obj_set_size(add_screen, 250, 280);
        lv_obj_set_scrollbar_mode(add_screen, LV_SCROLLBAR_MODE_OFF);
        //lv_style_set_clip_corner(&circle, true);
        lv_obj_add_style(add_screen, &circle, 0);
        // set watch face bg color
        lv_obj_set_style_bg_color(add_screen, lv_color_black(), 0);

        lv_obj_t *add_btn = lv_btn_create(add_screen);
        lv_obj_set_size(add_btn, 209, 209); // 设置按钮大小
        lv_obj_center(add_btn);
        lv_obj_set_style_border_width(add_btn, 0, 0);

        // 设置按钮背景图片
        // lv_obj_t * add_btn_img = lv_img_create(add_btn);
        // LV_IMAGE_DECLARE(Add_Face);  // 声明加号按钮图片
        // lv_img_set_src(add_btn_img, &Add_Face);
        // lv_obj_center(add_btn_img);

        lv_obj_t *AddlLabel = lv_label_create(add_screen);
        lv_label_set_text(AddlLabel, "Add new watch face");
        lv_obj_set_style_text_color(AddlLabel, lv_color_white(), 0);
        lv_obj_set_style_border_width(add_screen, 0, 0);
        lv_obj_align_to(AddlLabel, add_screen, LV_ALIGN_CENTER, 0, 0);

      

        lv_obj_align_to(add_screen, digitalDial, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    }
    if (!strcmp(str, "digital_dial"))
    {
        digitalDial = lv_obj_create(chooseDial);
        lv_obj_set_style_bg_color(digitalDial, lv_color_hex(0x3A7BD5), 0); // new code
        lv_obj_set_scrollbar_mode(digitalDial, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_size(digitalDial, 250, 280);
         lv_obj_add_style(digitalDial,&circle,0);//new code
        lv_obj_align_to(digitalDial, clockDial, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
        lv_obj_add_flag(digitalDial, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(digitalDial, click_event_cb, LV_EVENT_CLICKED, "digital_dial");

        // lv_obj_t* digitalImg = lv_image_create(digitalDial);
        // LV_IMAGE_DECLARE(digital_dial_choose);
        // lv_image_set_src(digitalImg, &digital_dial_choose);
        // lv_obj_center(digitalImg);

        lv_obj_t *digitalLabel = lv_label_create(digitalDial);
        lv_label_set_text(digitalLabel, "DigitalDial");
        lv_obj_set_style_text_color(digitalLabel, lv_color_white(), 0);
        lv_obj_set_style_border_width(digitalDial, 0, 0);
        lv_obj_align_to(digitalLabel, digitalDial, LV_ALIGN_CENTER, 0, 0);

        clockDial = lv_obj_create(chooseDial);
        lv_obj_set_style_bg_color(clockDial, lv_color_hex(0x030301), 0);
        lv_obj_set_scrollbar_mode(clockDial, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_size(clockDial, 250, 280);
        lv_obj_add_style(clockDial,&circle,0);//new code
        lv_obj_center(clockDial);
        lv_obj_add_flag(clockDial, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(clockDial, click_event_cb, LV_EVENT_CLICKED, "clock_dial");

        // lv_obj_t* clockImg = lv_image_create(clockDial);
        // LV_IMAGE_DECLARE(clock_dial_choose);
        // lv_image_set_src(clockImg, &clock_dial_choose);
        // lv_obj_center(clockImg);

        lv_obj_t *clockLabel = lv_label_create(clockDial);
        lv_label_set_text(clockLabel, "ClockDial");
        lv_obj_set_style_text_color(clockLabel, lv_color_white(), 0);
        lv_obj_set_style_border_width(clockDial, 0, 0);
        lv_obj_align_to(clockLabel, clockDial, LV_ALIGN_CENTER, 0, 0); // new code

        add_screen = lv_obj_create(chooseDial);
        // lv_obj_add_style(add_screen, &style_face, 0);
        lv_obj_set_size(add_screen, 250, 280);
        lv_obj_set_scrollbar_mode(add_screen, LV_SCROLLBAR_MODE_OFF);
        //lv_style_set_clip_corner(&circle, true);
        lv_obj_add_style(add_screen, &circle, 0);
        // set watch face bg color
        lv_obj_set_style_bg_color(add_screen, lv_color_black(), 0);

        lv_obj_t *add_btn = lv_btn_create(add_screen);
        lv_obj_set_size(add_btn, 209, 209); // 设置按钮大小
        lv_obj_center(add_btn);
        lv_obj_set_style_border_width(add_btn, 0, 0);

        // 设置按钮背景图片
        // lv_obj_t * add_btn_img = lv_img_create(add_btn);
        // LV_IMAGE_DECLARE(Add_Face);  // 声明加号按钮图片
        // lv_img_set_src(add_btn_img, &Add_Face);
        // lv_obj_center(add_btn_img);

        lv_obj_t *AddlLabel = lv_label_create(add_screen);
        lv_label_set_text(AddlLabel, "Add new watch face");
        lv_obj_set_style_text_color(AddlLabel, lv_color_white(), 0);
        lv_obj_set_style_border_width(add_screen, 0, 0);
        lv_obj_align_to(AddlLabel, add_screen, LV_ALIGN_CENTER, 0, 0);

       

        lv_obj_align_to(add_screen, digitalDial, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    }

    /*origin logic is simulate add_page*/
    // lv_obj_t* plusBtn = lv_button_create(chooseDial);
    // lv_obj_set_size(plusBtn, lv_obj_get_width(digitalDial), lv_obj_get_height(digitalDial));
    // lv_obj_set_style_bg_color(plusBtn, lv_color_black(), 0);
    // lv_obj_align_to(plusBtn, digitalDial, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    // lv_obj_t* plusLabel = lv_label_create(plusBtn);
    // lv_label_set_text(plusLabel, LV_SYMBOL_PLUS);
    // lv_obj_set_style_text_font(plusLabel, &lv_font_montserrat_32, 0);
    // lv_obj_center(plusLabel);

    lv_obj_update_snap(chooseDial, LV_ANIM_ON);
}