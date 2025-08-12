#include <time.h>
#include "utils.h"
#include "../include/utils.h"

#define LV_CIRCLE_WATCH 455

lv_obj_t* create_watch_scr(void)
{
    LV_LOG_USER("create_watch_scr in");
    lv_obj_t* watchScr = lv_obj_create(lv_scr_act());
    lv_obj_set_size(watchScr, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_center(watchScr);
    lv_obj_set_style_bg_color(watchScr, lv_color_black(), 0);
    lv_obj_set_style_radius(watchScr, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(watchScr, 0, 0);
    lv_obj_set_style_pad_all(watchScr, 0, 0);
    lv_obj_set_style_clip_corner(watchScr, true, 0);
    lv_obj_set_scrollbar_mode(watchScr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(watchScr, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(watchScr, LV_SCROLL_SNAP_CENTER);
    lv_obj_clear_flag(watchScr, LV_OBJ_FLAG_SCROLLABLE);
    return watchScr;
}