#include "/home/ts/Code/0717-01/apps/packages/apps/watch/launcher/src/../include/app_list.h"

#include "../../timer/include/timer_main.h"
#include "/home/ts/Code/0717-01/apps/packages/apps/watch/launcher/src/../../main_page/include/main_page.h"
#include "string.h"

lv_obj_t *appListCenterScreen = NULL;
struct MESSAGE_CENTER *appListCenterObj = NULL;

 

static void special_icon_click_handler(lv_event_t *e)
{
    //new code
    strcpy(mpd->current_page,INSOMEONEAPP_PAGE);
    //new code


    lv_event_code_t code = lv_event_get_code(e);
    uint32_t flag= lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED)
    {
        
        LV_LOG_USER("special_icon_click in flag %d \n", flag);
        
        if (flag == 1)
        {
            LV_LOG_USER("should go to timer_app_start() \n");
            timer_app_start();
        }
        else if (flag == 2)
        {
             LV_LOG_USER("should go to clock() \n");
            //clock_app_start();
        }
        else
        {
            LV_LOG_USER("The icon you clicked is undefined\n");

            // lv_obj_t *mbox1 = lv_msgbox_create(lv_screen_active());
            // lv_msgbox_add_title(mbox1, "Error");
            // lv_msgbox_add_text(mbox1, "Application didn't exit！!!!!");
            // lv_msgbox_add_close_button(mbox1);
            // lv_obj_center(mbox1);
        }
    }
}

static lv_obj_t *create_item(lv_obj_t *parent, lv_coord_t size, lv_coord_t cx, lv_coord_t cy, int row, int col)

{

    lv_obj_t *item = lv_label_create(parent);

    lv_obj_set_size(item, size, size);

    lv_obj_set_style_radius(item, LV_RADIUS_CIRCLE, 0);

    lv_obj_set_style_bg_opa(item, LV_OPA_COVER, 0);

    lv_obj_set_pos(item, cx - size / 2, cy - size / 2);

    lv_label_set_text(item, "");

    if (row == 2 && col == 1)//turn to timer
    {

        lv_obj_set_style_bg_color(item, lv_color_hex(0x3498db), LV_PART_MAIN);

        lv_obj_t *icon = lv_label_create(item);
        lv_label_set_text(icon, LV_SYMBOL_BELL);
       
        lv_obj_remove_style(icon, NULL, LV_PART_MAIN);
        lv_obj_set_style_text_color(icon, lv_color_white(), 0);
        // lv_obj_set_style_transform_zoom(icon, 256 * 4, 0);
        lv_obj_center(icon);
        uint32_t turn_to_timer =1;
       
        lv_obj_add_event_cb(item, special_icon_click_handler, LV_EVENT_CLICKED, turn_to_timer);

        // lv_obj_set_style_bg_color(item, lv_color_hex(0x2980b9), LV_STATE_PRESSED);
        // lv_obj_set_style_transform_zoom(item, 110, LV_STATE_PRESSED);
    }
    else if (row == 2 && col == 2) //turn to clock
    {
        lv_obj_set_style_bg_color(item, lv_color_hex(0x3498db), LV_PART_MAIN);

        lv_obj_t *icon = lv_label_create(item);
        lv_label_set_text(icon, LV_SYMBOL_AUDIO);
       
        lv_obj_remove_style(icon, NULL, LV_PART_MAIN);
        lv_obj_set_style_text_color(icon, lv_color_white(), 0);
        lv_obj_center(icon);

        uint32_t turn_to_clock =2;
        lv_obj_add_event_cb(item, special_icon_click_handler, LV_EVENT_CLICKED, turn_to_clock);

     
    } else {
       
        uint32_t show_msg_box =3;
        //in this case,shouldn't do event_cb,else will need Right Slide twice
        // lv_obj_add_event_cb(item, special_icon_click_handler, LV_EVENT_CLICKED, show_msg_box);

    }

    lv_color_t random_color = lv_color_make(rand() % 256, rand() % 256, rand() % 256);

    lv_obj_set_style_bg_color(item, random_color, LV_PART_MAIN);

    lv_obj_set_style_border_width(item, 5, LV_PART_MAIN);

    lv_obj_set_style_border_color(item, lv_color_hex(0xffffff), LV_PART_MAIN);

    return item;
}

static void update_item(lv_obj_t *item, lv_coord_t size, lv_coord_t cx, lv_coord_t cy)

{

    if (size < 1)
    {

        size = 1;
    }

    lv_obj_set_size(item, size, size);

    lv_obj_set_pos(item, cx - size / 2, cy - size / 2);
}

static void child_event_cb(lv_event_t *e);

static void slide_event_cb(lv_event_t *e);

static void transform_animation_cb(void *var, int32_t v);

static void snap_animation_cb(void *var, int32_t v);

static void fling_animation_cb(void *var, int32_t v);

static void add_scroll_distance(int dis, panel_user_data_t *panel_data);

static void change_order(lv_obj_t *panel, int source, int target);

static void range_animation_cb(void *var, int32_t v);

static int find_item_index_under(int x, int y, panel_user_data_t *panel_data);

static void show_back_gesture(panel_user_data_t *panel_data);

static bool hide_back_gesture(panel_user_data_t *panel_data);

static void update_back_gesture(int dis, panel_user_data_t *panel_data);

static void cleanup_panel_data(panel_user_data_t *panel_data)

{

    if (panel_data == NULL)
    {

        return;
    }

    // 清理网格数据

    if (panel_data->grid_items != NULL)
    {

        for (int i = 0; i < panel_data->row_count; i++)
        {

            if (panel_data->grid_items[i] != NULL)
            {

                free(panel_data->grid_items[i]);
            }
        }

        free(panel_data->grid_items);
    }

    // 清理排序动画数据

    if (panel_data->order_data != NULL)
    {

        if (panel_data->order_data->froms != NULL)
        {

            free(panel_data->order_data->froms);
        }

        if (panel_data->order_data->tos != NULL)
        {

            free(panel_data->order_data->tos);
        }

        free(panel_data->order_data);
    }

    // 清理返回手势Canvas

    if (panel_data->back_canvas != NULL)
    {

        lv_obj_del(panel_data->back_canvas);

        panel_data->back_canvas = NULL;
    }

    free(panel_data);
}

static void panel_delete_cb(lv_event_t *e)

{

    lv_obj_t *panel = lv_event_get_target(e);

    panel_user_data_t *panel_data = (panel_user_data_t *)lv_obj_get_user_data(panel);

    // 清理子项的用户数据

    for (int i = 0; i < lv_obj_get_child_cnt(panel); i++)
    {

        lv_obj_t *child = lv_obj_get_child(panel, i);

        drag_user_data_t *drag_data = (drag_user_data_t *)lv_obj_get_user_data(child);

        if (drag_data != NULL)
        {

            free(drag_data);
        }
    }

    cleanup_panel_data(panel_data);
}

static void panel_guest_release_cb(lv_event_t *e)
{
    lv_obj_t *panel = lv_event_get_target(e);
    panel_user_data_t *panel_data = (panel_user_data_t *)malloc(sizeof(panel_user_data_t));
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_RIGHT)
    {
        /* code */
        printf("panel_guest_release_cb right!");

        lv_anim_t anim;  // 动画对象
            lv_anim_init(&anim);  // 初始化动画
            lv_anim_set_var(&anim, panel);  // 设置动画作用的对象（面板）

            // 根据当前全局变换因子判断动画方向（展开/折叠）
            lv_anim_set_values(&anim, 0, 1000);  // 从0到1000：展开
            lv_obj_add_flag(panel,LV_OBJ_FLAG_GESTURE_BUBBLE);

            lv_anim_set_exec_cb(&anim, transform_animation_cb);  // 动画执行回调
            lv_anim_set_path_cb(&anim, lv_anim_path_linear);  // 线性动画路径
            lv_anim_set_time(&anim, 450);  // 动画时长450毫秒
            lv_anim_start(&anim);  // 启动动画

         panel_data->last_point_y = -1;  // 重置上一次触摸Y坐标（标记为未初始化）
    }

}

void app_grid(lv_obj_t *appListCenterScreen)
{
    lv_obj_t *panel = lv_obj_create(appListCenterScreen);

    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_outline_width(panel, 0, 0);

    LV_LOG_USER("app_grid W: %d, H: %d ", lv_obj_get_width(panel), lv_obj_get_height(panel));

    // 创建并初始化面板用户数据

    panel_user_data_t *panel_data = (panel_user_data_t *)malloc(sizeof(panel_user_data_t));

    memset(panel_data, 0, sizeof(panel_user_data_t));

    panel_data->row_count = 20;

    panel_data->grid_items = build_gird_items(panel_data->row_count);

    panel_data->last_point_y = -1;

    panel_data->no_slide_flag = true;

    lv_obj_set_user_data(panel, panel_data);

    // 面板基础设置

    lv_obj_set_size(panel, screen_size, screen_size);

    lv_obj_center(panel);

    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN);

    lv_obj_set_style_radius(panel, LV_RADIUS_CIRCLE, 0);

    lv_obj_set_style_clip_corner(panel, true, 0);

    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);

    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    // lv_obj_clear_flag(panel, LV_OBJ_FLAG_SNAPPABLE);

    // 添加事件回调

    lv_obj_add_event_cb(panel, panel_guest_release_cb, LV_EVENT_GESTURE, NULL);

    lv_obj_add_event_cb(panel, slide_event_cb, LV_EVENT_PRESSING, NULL);

    lv_obj_add_event_cb(panel, slide_event_cb, LV_EVENT_RELEASED, NULL);

    lv_obj_add_event_cb(panel, slide_event_cb, LV_EVENT_LONG_PRESSED, NULL);

    // 初始化网格

    transform_grid(panel_data->grid_items, panel_data->row_count, 0);

    // 创建网格项

    for (int i = 0; i < panel_data->row_count; i++)
    {

        for (int j = 0; j < 3; j++)
        {

            lv_obj_t *child = create_item(panel,

                                          panel_data->grid_items[i][j].size,

                                          panel_data->grid_items[i][j].center_x,

                                          panel_data->grid_items[i][j].center_y + panel_data->total_scroll_distance,
                                          i,
                                          j);

            // 创建并初始化拖拽用户数据

            drag_user_data_t *drag_data = (drag_user_data_t *)malloc(sizeof(drag_user_data_t));

            memset(drag_data, 0, sizeof(drag_user_data_t));

            drag_data->pressed_item_index = -1;

            drag_data->last_pressed_point_x = -1;

            drag_data->last_pressed_point_y = -1;

            lv_obj_set_user_data(child, drag_data);

            // 添加事件回调

            lv_obj_add_flag(child, LV_OBJ_FLAG_CLICKABLE);

            lv_obj_add_event_cb(child, child_event_cb, LV_EVENT_LONG_PRESSED, NULL);

            lv_obj_add_event_cb(child, child_event_cb, LV_EVENT_PRESSING, NULL);

            lv_obj_add_event_cb(child, child_event_cb, LV_EVENT_RELEASED, NULL);

            // lv_obj_add_event_cb(child, child_event_cb, LV_EVENT_ALL, NULL);
        }
    }

    // 添加删除事件回调，用于清理资源

    lv_obj_add_event_cb(panel, panel_delete_cb, LV_EVENT_DELETE, NULL);
}

static int find_item_index_under(int x, int y, panel_user_data_t *panel_data)

{

    y = y - panel_data->total_scroll_distance;

    for (int i = 0; i < panel_data->row_count; i++)
    {

        for (int j = 0; j < 3; j++)
        {

            if (abs(panel_data->grid_items[i][j].center_x - x) <= app_item_size / 2 &&

                abs(panel_data->grid_items[i][j].center_y - y) <= app_item_size / 2)
            {

                return i * 3 + j;
            }
        }
    }

    return -1;
}

static int count = 0;

static void child_event_cb(lv_event_t *e)

{

    lv_obj_t *item = lv_event_get_target(e);

    lv_obj_t *panel = lv_obj_get_parent(item);

    panel_user_data_t *panel_data = (panel_user_data_t *)lv_obj_get_user_data(panel);

    drag_user_data_t *drag_data = (drag_user_data_t *)lv_obj_get_user_data(item);

    lv_event_code_t code = lv_event_get_code(e);

    lv_indev_t *indev = lv_event_get_indev(e);

    lv_point_t point;

    lv_indev_get_point(indev, &point);

    if (drag_data->last_pressed_point_x < 0)
    {

        drag_data->last_pressed_point_x = point.x;

        drag_data->last_pressed_point_y = point.y;

        printf("child_event_cb skip. 0\n");

        slide_event_cb(e);

        return;
    }

    lv_coord_t diff_x = point.x - drag_data->last_pressed_point_x;

    lv_coord_t diff_y = point.y - drag_data->last_pressed_point_y;

    drag_data->last_pressed_point_x = point.x;

    drag_data->last_pressed_point_y = point.y;

    if (diff_x != 0 || diff_y != 0)
    {

        drag_data->slide_flag = true;
    }

    if (global_transform_factor != 0)
    {

        printf("child_event_cb skip. 1\n");

        slide_event_cb(e);

        return;
    }

    if (code == LV_EVENT_PRESSING && drag_data->pressed_item == NULL)
    {

        printf("child_event_cb skip. 2\n");

        slide_event_cb(e);

        return;
    }

    if (code == LV_EVENT_RELEASED)
    {

        if (drag_data->pressed_item != NULL && drag_data->pressed_item_index >= 0)
        {

            int target = find_item_index_under(point.x, point.y, panel_data);

            if (target < 0)
            {

                lv_obj_move_to_index(drag_data->pressed_item, drag_data->pressed_item_index);
            }
            else
            {

                lv_obj_set_style_opa(drag_data->pressed_item, 0, LV_PART_MAIN);

                lv_obj_move_to_index(drag_data->pressed_item, drag_data->pressed_item_index);

                lv_obj_set_style_border_color(drag_data->pressed_item, lv_color_hex(0xffffff), LV_PART_MAIN);

                change_order(drag_data->pressed_item->parent, drag_data->pressed_item_index, target);
            }
        }

        drag_data->slide_flag = false;

        drag_data->pressed_item_index = -1;

        drag_data->pressed_item = NULL;

        drag_data->last_pressed_point_x = -1;

        drag_data->last_pressed_point_y = -1;

        printf("child_event_cb skip. 3\n");

        slide_event_cb(e);

        return;
    }

    if (drag_data->pressed_item == NULL && drag_data->slide_flag)
    {

        printf("child_event_cb skip. 4\n");

        slide_event_cb(e);

        return;
    }

    if (code == LV_EVENT_LONG_PRESSED && !drag_data->slide_flag)
    {

        drag_data->pressed_item_index = lv_obj_get_index(item);

        drag_data->pressed_item = item;

        drag_data->first_pressed_point_x = point.x;

        drag_data->first_pressed_point_y = point.y;

        lv_coord_t size = lv_obj_get_width(drag_data->pressed_item);

        lv_coord_t center_x = lv_obj_get_x(drag_data->pressed_item) + size / 2;

        lv_coord_t center_y = lv_obj_get_y(drag_data->pressed_item) + size / 2;

        drag_data->default_center_x = center_x;

        drag_data->default_center_y = center_y;

        printf("child_event_cb skip. 5\n");

        return;
    }

    lv_coord_t size = lv_obj_get_width(drag_data->pressed_item);

    lv_coord_t center_x = lv_obj_get_x(drag_data->pressed_item) + size / 2;

    lv_coord_t center_y = lv_obj_get_y(drag_data->pressed_item) + size / 2;

    lv_obj_move_foreground(drag_data->pressed_item);

    lv_obj_set_style_border_color(drag_data->pressed_item, lv_color_hex(0xff0000), LV_PART_MAIN);

    lv_coord_t cx = drag_data->default_center_x + (point.x - drag_data->first_pressed_point_x);

    lv_coord_t cy = drag_data->default_center_y + (point.y - drag_data->first_pressed_point_y);

    update_item(drag_data->pressed_item, size, cx, cy);
}

static void slide_event_cb(lv_event_t *e)

{

    lv_obj_t *item = lv_event_get_target(e);

    lv_obj_t *panel = (lv_obj_get_width(item) != screen_size) ? item->parent : item;

    panel_user_data_t *panel_data = (panel_user_data_t *)lv_obj_get_user_data(panel);

    lv_event_code_t code = lv_event_get_code(e);

    lv_indev_t *indev = lv_event_get_indev(e);

    lv_point_t point;

    lv_indev_get_point(indev, &point);

    // 处理返回手势

    if (!panel_data->in_back_touch)
    {

        if (point.x < (app_item_size / 3))
        {

            panel_data->in_back_touch = true;

            panel_data->back_touch_down_x = point.x;

            // show_back_gesture(panel_data);

            return;
        }
    }

    if (code == LV_EVENT_RELEASED)

    {

        printf("edge_event_cb_clear %d\n", panel_data->back_touch_down_x);

        // bool back_available = hide_back_gesture(panel_data);
        bool back_available = false;

        if (back_available && global_transform_factor == 0)

        {

            lv_anim_t anim;

            lv_anim_init(&anim);

            lv_anim_set_var(&anim, panel);

            lv_anim_set_values(&anim, 0, 1000);

            lv_anim_set_exec_cb(&anim, transform_animation_cb);

            lv_anim_set_path_cb(&anim, lv_anim_path_linear);

            lv_anim_set_time(&anim, 450);

            lv_anim_start(&anim);
        }

        panel_data->in_back_touch = false;
    }

    if (panel_data->in_back_touch)

    {

        printf("edge_event_cb %d %d\n", (point.x - panel_data->back_touch_down_x), point.x);

        update_back_gesture(point.x - panel_data->back_touch_down_x, panel_data);

        return;
    }

    if (code == LV_EVENT_LONG_PRESSED)

    {

        printf("LV_EVENT_LONG_PRESSED.\n");

        if (panel_data->no_slide_flag)

        {

            lv_anim_t anim;

            lv_anim_init(&anim);

            lv_anim_set_var(&anim, panel);

            if (global_transform_factor == 0.0f)

            {

                lv_anim_set_values(&anim, 0, 1000);

                lv_obj_add_flag(panel, LV_OBJ_FLAG_GESTURE_BUBBLE);
            }

            else

            {

                lv_anim_set_values(&anim, 1000, 0);

                lv_obj_remove_flag(panel, LV_OBJ_FLAG_GESTURE_BUBBLE);
            }

            lv_anim_set_exec_cb(&anim, transform_animation_cb);

            lv_anim_set_path_cb(&anim, lv_anim_path_linear);

            lv_anim_set_time(&anim, 450);

            lv_anim_start(&anim);
        }

        panel_data->last_point_y = -1;

        return;
    }

    if (code == LV_EVENT_RELEASED)

    {

        panel_data->fling_start_diff_y = panel_data->slide_diff_y;

        printf("SPEED = %d\n", panel_data->fling_start_diff_y);

        panel_data->last_point_y = -1;

        panel_data->slide_diff_y = 0;

        panel_data->no_slide_flag = true;

        lv_anim_init(&panel_data->fling_anim);

        lv_anim_set_var(&panel_data->fling_anim, panel);

        lv_anim_set_values(&panel_data->fling_anim, 0, 1000);

        lv_anim_set_exec_cb(&panel_data->fling_anim, fling_animation_cb);

        lv_anim_set_path_cb(&panel_data->fling_anim, lv_anim_path_ease_out);

        int fling_time = abs(panel_data->fling_start_diff_y) * 10; // lv_anim_speed_to_time(10, 0, abs(panel_data->fling_start_diff_y));

        lv_anim_set_time(&panel_data->fling_anim, fling_time);

        lv_anim_start(&panel_data->fling_anim);

        return;
    }

    if (panel_data->last_point_y < 0)

    {

        panel_data->last_point_y = point.y;

        return;
    }

    panel_data->slide_diff_y = point.y - panel_data->last_point_y;

    panel_data->last_point_y = point.y;

    add_scroll_distance(panel_data->slide_diff_y, panel_data);

    if (panel_data->slide_diff_y == 0)

    {

        return;
    }

    panel_data->no_slide_flag = false;

    lv_anim_del(&panel_data->fling_anim, fling_animation_cb);

    lv_anim_del(&panel_data->snap_anim, snap_animation_cb);

    float scroll_factor = panel_data->total_scroll_distance * 1.0 / app_item_size;

    transform_grid(panel_data->grid_items, panel_data->row_count, -scroll_factor);

    printf("POINT(%d, %d), DIFF(%d), FACTOR(%4.3f).\n", point.x, point.y, panel_data->slide_diff_y, -scroll_factor);

    for (size_t i = 0; i < lv_obj_get_child_cnt(panel); i++)

    {

        lv_obj_t *child = lv_obj_get_child(panel, i);

        lv_coord_t child_y = lv_obj_get_y(child);

        lv_coord_t child_h = lv_obj_get_height(child);

        grid_item_info info = panel_data->grid_items[i / 3][i % 3];

        update_item(child, info.size, info.center_x, info.center_y + panel_data->total_scroll_distance);
    }
}

static void transform_animation_cb(void *var, int32_t v)

{

    lv_obj_t *panel = (lv_obj_t *)var;

    panel_user_data_t *panel_data = (panel_user_data_t *)lv_obj_get_user_data(panel);

    global_transform_factor = v * 1.0f / 1000;

    float scroll_factor = panel_data->total_scroll_distance * 1.0f / app_item_size;

    transform_grid(panel_data->grid_items, panel_data->row_count, -scroll_factor);

    for (size_t i = 0; i < lv_obj_get_child_cnt(panel); i++)
    {

        lv_obj_t *child = lv_obj_get_child(panel, i);

        grid_item_info info = panel_data->grid_items[i / 3][i % 3];

        update_item(child, info.size, info.center_x,

                    info.center_y + panel_data->total_scroll_distance);
    }
}

static void snap_animation_cb(void *var, int32_t v)

{

    lv_obj_t *panel = (lv_obj_t *)var;

    panel_user_data_t *panel_data = (panel_user_data_t *)lv_obj_get_user_data(panel);

    float anim_value = v * 1.0f / 1000;

    panel_data->snap_distance_consumed = panel_data->snap_distance * anim_value;

    add_scroll_distance(panel_data->snap_start_distance +

                            panel_data->snap_distance_consumed - panel_data->total_scroll_distance,
                        panel_data);

    float scroll_factor = panel_data->total_scroll_distance * 1.0f / app_item_size;

    transform_grid(panel_data->grid_items, panel_data->row_count, -scroll_factor);

    for (size_t i = 0; i < lv_obj_get_child_cnt(panel); i++)
    {

        lv_obj_t *child = lv_obj_get_child(panel, i);

        grid_item_info info = panel_data->grid_items[i / 3][i % 3];

        update_item(child, info.size, info.center_x,

                    info.center_y + panel_data->total_scroll_distance);
    }
}

static void fling_animation_cb(void *var, int32_t v)

{

    lv_obj_t *panel = (lv_obj_t *)var;

    panel_user_data_t *panel_data = (panel_user_data_t *)lv_obj_get_user_data(panel);

    float anim_value = v * 1.0f / 1000;

    add_scroll_distance(panel_data->fling_start_diff_y * (1 - anim_value), panel_data);

    float scroll_factor = panel_data->total_scroll_distance * 1.0f / app_item_size;

    transform_grid(panel_data->grid_items, panel_data->row_count, -scroll_factor);

    for (size_t i = 0; i < lv_obj_get_child_cnt(panel); i++)
    {

        lv_obj_t *child = lv_obj_get_child(panel, i);

        grid_item_info info = panel_data->grid_items[i / 3][i % 3];

        update_item(child, info.size, info.center_x,

                    info.center_y + panel_data->total_scroll_distance);
    }

    if (anim_value == 1)
    {

        int distance = (-panel_data->total_scroll_distance) % app_item_size;

        if (distance == 0)
        {

            return;
        }

        panel_data->snap_start_distance = panel_data->total_scroll_distance;

        panel_data->snap_distance_consumed = 0;

        panel_data->snap_distance = (distance > app_item_size / 2) ?

                                                                   distance - app_item_size
                                                                   : distance;

        lv_anim_init(&panel_data->snap_anim);

        lv_anim_set_var(&panel_data->snap_anim, panel);

        lv_anim_set_values(&panel_data->snap_anim, 0, 1000);

        lv_anim_set_exec_cb(&panel_data->snap_anim, snap_animation_cb);

        lv_anim_set_path_cb(&panel_data->snap_anim, lv_anim_path_ease_in);

        int snap_time = abs(panel_data->snap_distance) * 10; // lv_anim_speed_to_time(50, 0, panel_data->snap_distance);

        lv_anim_set_time(&panel_data->snap_anim, snap_time);

        lv_anim_start(&panel_data->snap_anim);
    }
}

static void add_scroll_distance(int dis, panel_user_data_t *panel_data)

{

    panel_data->total_scroll_distance += dis;

    int max = app_item_size * 2;

    int min = max - app_item_size * (panel_data->row_count - 1);

    if (panel_data->total_scroll_distance < min)
    {

        panel_data->total_scroll_distance = min;
    }
    else if (panel_data->total_scroll_distance > max)
    {

        panel_data->total_scroll_distance = max;
    }
}

static void range_animation_cb(void *var, int32_t v)

{

    lv_obj_t *panel = (lv_obj_t *)var;

    panel_user_data_t *panel_data = (panel_user_data_t *)lv_obj_get_user_data(panel);

    order_animation_data_t *order_data = panel_data->order_data;

    float anim_value = v * 1.0f / 1000;

    int size = abs(order_data->target - order_data->source) + 1;

    lv_obj_t *source_item = NULL;

    for (int i = 0; i < size; i++)
    {

        int index = order_data->source + (order_data->source < order_data->target ? i : -i);

        lv_obj_t *child = lv_obj_get_child(panel, index);

        grid_item_info from = order_data->froms[i];

        grid_item_info to = order_data->tos[i];

        update_item(child,

                    from.size + (to.size - from.size) * anim_value,

                    from.center_x + (to.center_x - from.center_x) * anim_value,

                    from.center_y + (to.center_y - from.center_y) * anim_value);

        if (i == 0 && anim_value > 0.1f)
        {

            lv_obj_set_style_opa(child, 255, LV_PART_MAIN);
        }

        if (anim_value == 1.0f)
        {

            if (i != 0)
            {

                int pre_index = index + (order_data->source < order_data->target ? -1 : 1);

                lv_obj_move_to_index(child, pre_index);
            }
            else
            {

                source_item = child;
            }
        }
    }

    if (anim_value == 1.0f && source_item != NULL)
    {

        lv_obj_move_to_index(source_item, order_data->target);
    }
}

static void change_order(lv_obj_t *panel, int source, int target)

{

    panel_user_data_t *panel_data = (panel_user_data_t *)lv_obj_get_user_data(panel);

    // 清理旧数据

    if (panel_data->order_data != NULL)
    {

        if (panel_data->order_data->froms != NULL)
        {

            free(panel_data->order_data->froms);
        }

        if (panel_data->order_data->tos != NULL)
        {

            free(panel_data->order_data->tos);
        }

        free(panel_data->order_data);
    }

    // 创建新的排序动画数据

    panel_data->order_data = (order_animation_data_t *)malloc(sizeof(order_animation_data_t));

    panel_data->order_data->source = source;

    panel_data->order_data->target = target;

    int size = abs(target - source) + 1;

    panel_data->order_data->froms = (grid_item_info *)malloc(size * sizeof(grid_item_info));

    panel_data->order_data->tos = (grid_item_info *)malloc(size * sizeof(grid_item_info));

    // 初始化数据

    for (int i = 0; i < size; i++)
    {

        int index = source < target ? source + i : source - i;

        lv_obj_t *child = lv_obj_get_child(panel, index);

        // 初始化起始位置

        panel_data->order_data->froms[i] = (grid_item_info){

            .size = lv_obj_get_width(child),

            .center_x = lv_obj_get_x(child) + lv_obj_get_width(child) / 2,

            .center_y = lv_obj_get_y(child) + lv_obj_get_height(child) / 2

        };

        // 初始化目标位置

        if (i == 0)
        {

            init_grid_item(&panel_data->order_data->tos[i], target);

            panel_data->order_data->tos[i].center_y += panel_data->total_scroll_distance;
        }
        else
        {

            int pre_index = index + (source < target ? -1 : 1);

            init_grid_item(&panel_data->order_data->tos[i], pre_index);

            panel_data->order_data->tos[i].center_y += panel_data->total_scroll_distance;
        }
    }

    // 启动动画

    lv_anim_init(&panel_data->range_anim);

    lv_anim_set_var(&panel_data->range_anim, panel);

    lv_anim_set_values(&panel_data->range_anim, 0, 1000);

    lv_anim_set_exec_cb(&panel_data->range_anim, range_animation_cb);

    lv_anim_set_path_cb(&panel_data->range_anim, lv_anim_path_linear);

    lv_anim_set_time(&panel_data->range_anim, 450);

    lv_anim_start(&panel_data->range_anim);
}

static void show_back_gesture(panel_user_data_t *panel_data)

{

    printf("show_back_gesture\n");

    if (panel_data->back_canvas != NULL)
    {

        return;
    }

    // static lv_color_t canvas_buffer[LV_CANVAS_BUF_SIZE_ALPHA_4BIT(CANVAS_WIDTH, CANVAS_HEIGHT)];

    // static lv_draw_buf_t canvas_buffer;

    LV_DRAW_BUF_DEFINE(canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_ARGB8888);

    panel_data->back_canvas = lv_canvas_create(appListCenterScreen);

    lv_obj_set_pos(panel_data->back_canvas, 0, (screen_size - CANVAS_HEIGHT) / 2);

    // lv_canvas_set_buffer(panel_data->back_canvas, &canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_A4);

    lv_canvas_set_draw_buf(panel_data->back_canvas, &canvas_buffer);

    lv_canvas_fill_bg(panel_data->back_canvas, lv_color_black(), LV_OPA_30);

    // lv_layer_t layer;

    // lv_canvas_init_layer(panel_data->back_canvas, &layer);

    // lv_vector_dsc_t * dsc = lv_vector_dsc_create(&layer);

    // lv_vector_path_t * path = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM);

    // lv_fpoint_t pts[] = {{10, 10}, {100, 100}, {10, 100}};

    // lv_vector_path_move_to(path, &pts[0]);

    // lv_vector_path_line_to(path, &pts[1]);

    // lv_vector_path_line_to(path, &pts[2]);

    // lv_vector_path_close(path);

    // lv_vector_dsc_set_fill_color(dsc, lv_color_make(0x00, 0x80, 0xff));

    // lv_vector_dsc_add_path(dsc, path);

    // lv_draw_vector(dsc);

    // lv_vector_path_delete(path);

    // lv_vector_dsc_delete(dsc);

    // lv_canvas_finish_layer(panel_data->back_canvas, &layer);

    // printf("show_back_gesture\n");

    // if (panel_data->back_canvas != NULL) {

    //     return;

    // }

    // static lv_color_t canvas_buffer[LV_CANVAS_BUF_SIZE_ALPHA_4BIT(CANVAS_WIDTH, CANVAS_HEIGHT)];

    // panel_data->back_canvas = lv_canvas_create(lv_scr_act());

    // lv_obj_set_pos(panel_data->back_canvas, 0, (screen_size - CANVAS_HEIGHT) / 2);

    // lv_canvas_set_buffer(panel_data->back_canvas, canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_ALPHA_4BIT);

    // lv_canvas_fill_bg(panel_data->back_canvas, lv_color_black(), LV_OPA_30);

    // lv_draw_arc_dsc_t arc_dsc;

    // lv_draw_arc_dsc_init(&arc_dsc);

    // arc_dsc.color = lv_color_black();

    // arc_dsc.width = DEFAULT_ARC_WIDTH + CANVAS_WIDTH * (1.0f - panel_data->back_progress);

    // lv_canvas_draw_arc(panel_data->back_canvas,

    //     CANVAS_WIDTH + DEFAULT_ARC_WIDTH - BACK_GESTURE_ARC_RADIUS,

    //     CANVAS_HEIGHT / 2, BACK_GESTURE_ARC_RADIUS, -90, 90, &arc_dsc);

    // arc_dsc.width = DEFAULT_ARC_WIDTH;

    // lv_canvas_draw_arc(panel_data->back_canvas,

    //     screen_size / 2, CANVAS_HEIGHT / 2,

    //     screen_size / 2 + arc_dsc.width, 90, -90, &arc_dsc);
}

static void update_back_gesture(int dis, panel_user_data_t *panel_data)

{

    if (panel_data->back_canvas == NULL)
    {

        return;
    }

    panel_data->back_progress = dis * 2.0f / screen_size;

    if (panel_data->back_progress > 1.0f)
    {

        panel_data->back_progress = 1.0f;
    }

    // lv_canvas_fill_bg(panel_data->back_canvas, lv_color_black(), LV_OPA_40);

    // lv_draw_arc_dsc_t arc_dsc;

    // lv_draw_arc_dsc_init(&arc_dsc);

    // arc_dsc.color = lv_color_black();

    // arc_dsc.width = DEFAULT_ARC_WIDTH + CANVAS_WIDTH * (1.0f - panel_data->back_progress);

    // lv_canvas_draw_arc(panel_data->back_canvas,

    //     CANVAS_WIDTH + DEFAULT_ARC_WIDTH - BACK_GESTURE_ARC_RADIUS,

    //     CANVAS_HEIGHT / 2, BACK_GESTURE_ARC_RADIUS, -90, 90, &arc_dsc);

    // arc_dsc.width = DEFAULT_ARC_WIDTH;

    // lv_canvas_draw_arc(panel_data->back_canvas,

    //     screen_size / 2, CANVAS_HEIGHT / 2,

    //     screen_size / 2 + arc_dsc.width, 90, -90, &arc_dsc);
}

static bool hide_back_gesture(panel_user_data_t *panel_data)

{

    printf("hide_back_gesture\n");

    bool back_available = false;

    if (panel_data->back_canvas != NULL)
    {

        lv_obj_del(panel_data->back_canvas);

        back_available = panel_data->back_progress > 0.5f;
    }

    panel_data->back_canvas = NULL;

    panel_data->back_progress = 0.0f;

    return back_available;
}

void apps_center_start(lv_obj_t *appListCenterScreen)
{
    LV_LOG_USER("apps_center_start in");
    lv_obj_set_size(appListCenterScreen, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_set_style_bg_color(appListCenterScreen, lv_color_hex(0x030301), LV_PART_MAIN);
    lv_obj_align(appListCenterScreen, LV_ALIGN_CENTER, 0, -LV_CIRCLE_WATCH);
    lv_obj_set_style_radius(appListCenterScreen, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_pad_all(appListCenterScreen, 0, 0);
    lv_obj_set_style_border_width(appListCenterScreen, 0, 0);
    lv_obj_set_style_outline_width(appListCenterScreen, 0, 0);
    if (appListCenterObj == NULL)
    {
        appListCenterObj = malloc(sizeof(struct APP_LIST_CENTER));
    }
   
   
    app_grid(appListCenterScreen);
}