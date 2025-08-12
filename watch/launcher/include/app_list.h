#ifndef __APP_LIST_H
	
#define __APP_LIST_H
	
	
#include <lvgl/lvgl.h>
	
#include "app_list_math.h"


#define LV_CIRCLE_WATCH 455

#define CANVAS_WIDTH 60
	
#define CANVAS_HEIGHT 200
	
#define BACK_GESTURE_ARC_RADIUS 200
	
#define DEFAULT_ARC_WIDTH 60

struct APP_LIST_CENTER {
    lv_obj_t *appListCenter;
    lv_obj_t *appListCenterLabel;
    lv_obj_t *appListCenterImage;
    lv_obj_t *appListCenterImage2;
};
	
// 排序动画的用户数据结构体
	
typedef struct {
	
    int source;
	
    int target;
	
    grid_item_info *froms;
	
    grid_item_info *tos;
	
} order_animation_data_t;
	
	
// Panel的用户数据结构体
	
typedef struct {
	
    int row_count;
	
    grid_item_info **grid_items;
	
    lv_coord_t slide_diff_y;
	
    lv_coord_t last_point_y;
	
    lv_coord_t total_scroll_distance;
	
    bool no_slide_flag;
	
    float snap_start_distance;
	
    float snap_distance_consumed;
	
    float snap_distance;
	
    lv_coord_t fling_start_diff_y;
	
    lv_anim_t snap_anim;
	
    lv_anim_t fling_anim;
	
    lv_anim_t range_anim;
	
    bool in_back_touch;
	
    int back_touch_down_x;
	
    lv_obj_t *back_canvas;
	
    float back_progress;
	
    order_animation_data_t *order_data;
	
} panel_user_data_t;
	
	
// 拖拽项的用户数据结构体
	
typedef struct {
	
    bool slide_flag;
	
    int pressed_item_index;
	
    lv_obj_t *pressed_item;
	
    lv_coord_t last_pressed_point_x;
	
    lv_coord_t last_pressed_point_y;
	
    lv_coord_t first_pressed_point_x;
	
    lv_coord_t first_pressed_point_y;
	
    lv_coord_t default_center_x;
	
    lv_coord_t default_center_y;
	
} drag_user_data_t;
	
	
// 初始化应用网格
void app_grid(lv_obj_t *appListCenterScreen);
void apps_center_start(lv_obj_t *appListCenter);
	
	
#endif // __APP_LIST_H