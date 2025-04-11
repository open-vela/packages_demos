#include "relatives_cal.h"

static struct relatives_cal_s g_rel_cal;
static lv_style_t style;

static const char * btnm_map[] = {
    "父", "夫", "子", "兄", "姐", "女", "<-", "清空" , "\n",
    "母", "妻", "女", "弟", "妹", "男", " "  , "计算" , ""  };


void relatives_cal_app_create(void)
{
    LV_FONT_DECLARE(lv_font_simsun_16_cjk);


    lv_style_set_text_font(&style, &lv_font_montserrat_16);  

    lv_obj_t *root = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 800, 500);
    lv_obj_center(root);

    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);

    /*Input textarea*/
    lv_obj_t * ta_input = lv_textarea_create(root);
    g_rel_cal.ui.input = ta_input;
    lv_obj_set_size(ta_input, LV_PCT(100), LV_PCT(40));

    lv_obj_set_style_text_font(ta_input, &lv_font_simsun_16_cjk, LV_PART_MAIN);

    lv_style_set_text_opa(&style, 150);

    lv_obj_set_style_bg_color(ta_input, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_color(ta_input, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(ta_input, 0, 0);
    lv_obj_set_style_border_width(ta_input, 0, 0);
    
    lv_textarea_set_cursor_click_pos(ta_input, false);
    lv_textarea_set_max_length(ta_input, 128);
    lv_textarea_set_align(ta_input, LV_TEXT_ALIGN_LEFT);
    lv_textarea_set_text(ta_input, "大家好");
    lv_style_set_text_opa(&style, 100);

    /*Calculator input panel*/
    lv_obj_t * btnm = lv_btnmatrix_create(root);
    g_rel_cal.ui.btnm = btnm;
    lv_obj_set_style_border_width(btnm, 0, 0);
    lv_obj_set_size(btnm, LV_PCT(100), LV_PCT(40));
    lv_obj_set_style_text_font(btnm,&lv_font_simsun_16_cjk,LV_PART_MAIN);
    lv_buttonmatrix_set_map(btnm, btnm_map);

}

