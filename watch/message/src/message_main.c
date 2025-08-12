#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/message.h"
#ifndef MESSAGE_LIST_H
#define MESSAGE_LIST_H

static lv_obj_t *main_page; // 主页面
static lv_obj_t *message_list_page;
const int32_t MAX_SHOW_MESSAGE_NUM = 2;


static lv_obj_t *message_list;
lv_obj_t *messageCenterScreen = NULL;
struct MESSAGE_CENTER *messageCenterObj = NULL;
static lv_anim_t slide_anim;
static lv_obj_t *sliding_btn = NULL;
static lv_obj_t *delete_box = NULL;
static lv_style_t style_blue_icon; // 在文件顶部声明


static void delete_confirm_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *btn = lv_event_get_user_data(e);

    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_1))
    { // 确认按钮
        // 获取按钮所在的列表
        lv_obj_t *list = lv_obj_get_parent(btn);
        lv_obj_del(btn); // 删除消息按钮

        // 可选：更新消息列表布局
        lv_obj_update_layout(list);
    }

    // 无论确认还是取消，都删除消息框
    lv_obj_t *mbox = lv_obj_get_parent(obj);
    lv_obj_del(mbox);
}

static void demo_message_event_handler(lv_event_t *e) // 点开后显示是否删除消息的回调
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    // if(code == LV_EVENT_CLICKED) {
    //     LV_UNUSED(obj);
    //     LV_LOG_USER("Clicked: %s", lv_list_get_button_text(message_list_page, obj));
    // }
    if (code == LV_EVENT_LONG_PRESSED)
    { // 长按事件：LV_EVENT_LONG_PRESSED or  LV_EVENT_CLICKED
        // 创建消息框容器
        lv_obj_t *mbox = lv_obj_create(lv_layer_top());
        lv_obj_set_size(mbox, 300, 150);
        lv_obj_center(mbox);
        lv_obj_set_style_bg_opa(mbox, LV_OPA_90, 0);
        lv_obj_set_style_bg_color(mbox, lv_palette_main(LV_PALETTE_GREY), 0);
        lv_obj_set_style_radius(mbox, 10, 0);

        // 添加提示文本
        lv_obj_t *label = lv_label_create(mbox);
        lv_label_set_text(label, "Whether delete this message");
  

        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

        // 创建确认按钮
        lv_obj_t *confirm_btn = lv_btn_create(mbox);
        lv_obj_set_size(confirm_btn, 100, 40);
        lv_obj_align(confirm_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
        lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_USER_1); // 标记为确认按钮
        lv_obj_t *confirm_label = lv_label_create(confirm_btn);
        lv_label_set_text(confirm_label, "Confirm");

        lv_obj_center(confirm_label);

        // 创建取消按钮
        lv_obj_t *cancel_btn = lv_btn_create(mbox);
        lv_obj_set_size(cancel_btn, 100, 40);
        lv_obj_align(cancel_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
        lv_obj_t *cancel_label = lv_label_create(cancel_btn);
        lv_label_set_text(cancel_label, "Cancel");
  

        lv_obj_center(cancel_label);

        // 为两个按钮添加事件处理
        lv_obj_add_event_cb(confirm_btn, delete_confirm_handler, LV_EVENT_CLICKED, obj);
        lv_obj_add_event_cb(cancel_btn, delete_confirm_handler, LV_EVENT_CLICKED, obj);
    }
}

static void back_event_handler(lv_event_t *e)
{
    lv_obj_t *mbox = lv_obj_get_parent(lv_event_get_target(e));
    lv_obj_del(mbox);
}
static void message_show_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        lv_obj_t *label = lv_event_get_user_data(e);
        const char *text = (const char *)lv_obj_get_user_data(label);

        // 创建基础消息框
        lv_obj_t *mbox = lv_obj_create(lv_layer_top());
        lv_obj_set_size(mbox, 300, 150);
        lv_obj_center(mbox);
        lv_obj_set_style_bg_opa(mbox, LV_OPA_90, 0);
        lv_obj_set_style_bg_color(mbox, lv_palette_main(LV_PALETTE_GREY), 0);
        lv_obj_set_style_radius(mbox, 10, 0);

        // 添加标题
        lv_obj_t *title = lv_label_create(mbox);
        lv_label_set_text(title, "Confirm Message");

        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, -10);

        // 添加内容文本
        lv_obj_t *content = lv_label_create(mbox);
        lv_label_set_text(content, text);

        lv_obj_align(content, LV_ALIGN_CENTER, 0, -10);

        // 添加确定按钮
        lv_obj_t *ok_btn = lv_btn_create(mbox);
        lv_obj_set_size(ok_btn, 80, 30);
        lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
        lv_obj_t *ok_label = lv_label_create(ok_btn);
        lv_label_set_text(ok_label, "Confirm");
   
        lv_obj_center(ok_label);

        // 添加关闭事件
        lv_obj_add_event_cb(ok_btn, back_event_handler, LV_EVENT_CLICKED, mbox);
    }
}
static void swipe_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_point_t scroll_end;
    lv_obj_get_scroll_end(obj, &scroll_end);

    // 如果滑动距离不足，自动回弹到原始位置
    if (scroll_end.x > -50)
    {
        lv_obj_scroll_to_x(obj, 0, LV_ANIM_ON);
    }
    // 如果滑动足够，显示完整删除按钮
    else
    {
        lv_obj_scroll_to_x(obj, -100, LV_ANIM_ON);
    }
}
static void delete_message_handler(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *container = lv_event_get_user_data(e);

    // 删除整个滑动容器
    lv_obj_del(container);

    // 可选：更新消息列表布局
    lv_obj_update_layout(message_list);
}
// 添加滑动事件处理函数

// 添加手势事件处理函数
static void gesture_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_GESTURE)
    {
        lv_indev_t *indev = lv_indev_get_act();
        if (indev)
        {
            lv_dir_t dir = lv_indev_get_gesture_dir(indev);

            if (dir == LV_DIR_LEFT)
            {
                sliding_btn = obj;

                // 创建删除框（如果不存在）
                if (!delete_box)
                {
                    delete_box = lv_obj_create(lv_obj_get_parent(obj));
                    lv_obj_set_size(delete_box, 100, 100);
                    lv_obj_set_style_bg_color(delete_box, lv_palette_main(LV_PALETTE_RED), 0);
                    lv_obj_set_style_radius(delete_box, 0, 0);
                    lv_obj_add_flag(delete_box, LV_OBJ_FLAG_HIDDEN);

                    // 添加删除图标
                    lv_obj_t *trash_icon = lv_label_create(delete_box);
                    lv_label_set_text(trash_icon, LV_SYMBOL_TRASH);
                   
                    lv_obj_set_style_text_color(trash_icon, lv_color_white(), 0);
                    lv_obj_center(trash_icon);

                    // 添加删除事件
                    lv_obj_add_event_cb(delete_box, delete_message_handler, LV_EVENT_CLICKED, obj);
                }

                // 设置删除框位置并显示
                lv_obj_set_pos(delete_box,
                               lv_obj_get_x(obj) + lv_obj_get_width(obj) + 10,
                               lv_obj_get_y(obj));
                lv_obj_clear_flag(delete_box, LV_OBJ_FLAG_HIDDEN);

                // 创建滑动动画
                lv_anim_t a;
                lv_anim_init(&a);
                lv_anim_set_var(&a, obj);
                lv_anim_set_values(&a, lv_obj_get_x(obj), -110);
                lv_anim_set_time(&a, 300);
                lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
                lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
                lv_anim_start(&a);
            }
            else if (dir == LV_DIR_RIGHT && sliding_btn == obj)
            {
                // 右滑手势 - 移回原位
                lv_anim_t a;
                lv_anim_init(&a);
                lv_anim_set_var(&a, obj);
                lv_anim_set_values(&a, lv_obj_get_x(obj), 0);
                lv_anim_set_time(&a, 200);
                lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
                lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
                lv_anim_start(&a);

                // 隐藏删除框
                if (delete_box)
                {
                    lv_obj_add_flag(delete_box, LV_OBJ_FLAG_HIDDEN);
                }

                sliding_btn = NULL;
            }
        }
    }
}

static void init_message_list_page_bak(lv_obj_t *msgCenterScreen)
{

    LV_LOG_USER("init_message_list_page_bak in");

    lv_obj_set_style_bg_color(msgCenterScreen, lv_color_white(),0);
    lv_obj_t *title_label1 = lv_label_create(msgCenterScreen);
    lv_label_set_text(title_label1, "Message Notification");
    lv_obj_center(title_label1);
}

static void init_message_list_page(lv_obj_t *msgCenterScreen)
{
    // message_list_page = lv_obj_create(lv_screen_active());
    message_list_page = lv_obj_create(msgCenterScreen);
    lv_obj_set_size(message_list_page, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_center(message_list_page);
    lv_obj_set_style_bg_color(message_list_page, lv_color_black(), 0);
    lv_obj_set_style_radius(message_list_page, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(message_list_page, 0, 0);
    lv_obj_set_style_pad_all(message_list_page, 0, 0);
    lv_obj_set_style_clip_corner(message_list_page, true, 0);
    lv_obj_set_scrollbar_mode(message_list_page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(message_list_page, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(message_list_page, LV_SCROLL_SNAP_CENTER); // 对utils的设计的引用


    // 只允许上下滚动，禁用左右移动
    lv_obj_set_scroll_dir(message_list_page, LV_DIR_VER);
    lv_obj_clear_flag(message_list_page, LV_OBJ_FLAG_SCROLL_CHAIN_HOR); // 禁用横向链式滚动
    lv_obj_add_flag(message_list_page, LV_OBJ_FLAG_SCROLL_CHAIN_VER);   // 关闭垂直滚动链传递

    lv_obj_add_flag(message_list_page, LV_SCROLLBAR_MODE_OFF);                    // 使列表可滚动
    lv_obj_set_style_width(message_list_page, 0, LV_PART_SCROLLBAR);              // 设置滚动条宽度为0
    lv_obj_set_style_bg_opa(message_list_page, LV_OPA_TRANSP, LV_PART_SCROLLBAR); // 滚动条完全透明

    // 计划加的图像按钮
    // message1 = lv_imagebutton_create(message_list_page);
    // message2 = lv_imagebutton_create(message_list_page);

    message_list = lv_list_create(message_list_page);
    static lv_style_t no_border_style;
    lv_style_init(&no_border_style);
    lv_style_set_border_width(&no_border_style, 0);           // 设置边框宽度为0
    lv_style_set_border_opa(&no_border_style, LV_OPA_TRANSP); // 设置边框完全透明
    lv_obj_add_style(message_list, &no_border_style, 0);      // 应用无边框样式
    lv_obj_set_style_bg_color(message_list, lv_color_black(), 0);

    // 只允许上下滚动，禁用左右移动
    lv_obj_set_scroll_dir(message_list, LV_DIR_VER);
    lv_obj_clear_flag(message_list, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);           // 禁用横向链式滚动
    lv_obj_add_flag(message_list, LV_OBJ_FLAG_SCROLL_CHAIN_VER);             // 关闭垂直滚动链传递
    lv_obj_add_flag(message_list, LV_SCROLLBAR_MODE_OFF);                    // 使列表可滚动
    lv_obj_set_style_width(message_list, 0, LV_PART_SCROLLBAR);              // 设置滚动条宽度为0
    lv_obj_set_style_bg_opa(message_list, LV_OPA_TRANSP, LV_PART_SCROLLBAR); // 滚动条完全透明

    // 设置 message_list 的大小为圆内矩形，避免超出圆形边界
    // int padding = 20; // 适当留白，防止内容超出圆形
    lv_obj_set_size(message_list, 460, 585);

    // 只允许上下滚动，禁用左右移动
    lv_obj_set_scroll_dir(message_list, LV_DIR_VER);
    lv_obj_clear_flag(message_list, LV_OBJ_FLAG_SCROLL_CHAIN_HOR); // 禁用横向链式滚动
    lv_obj_set_scroll_snap_x(message_list, LV_SCROLL_SNAP_NONE);   // 禁用横向吸附
    lv_obj_set_scrollbar_mode(message_list, LV_SCROLLBAR_MODE_AUTO);

    lv_obj_center(message_list);
    // 创建一个容器作为自定义标题栏
    lv_obj_t *list_header_cont = lv_obj_create(message_list);
    lv_obj_remove_style_all(list_header_cont); // 移除默认样式
    lv_obj_set_size(list_header_cont, lv_pct(100), 50);
    lv_obj_set_flex_flow(list_header_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_row(list_header_cont, 0, 0);
    lv_obj_set_style_pad_column(list_header_cont, 8, 0);
    lv_obj_set_style_bg_opa(list_header_cont, LV_OPA_TRANSP, 0);
    

    // 添加一个空白按钮用于美观，占位在最上方
    lv_obj_t *spacer_btn;
    spacer_btn = lv_list_add_button(message_list, " ", "");
    lv_obj_set_width(spacer_btn, 400);
    lv_obj_set_height(spacer_btn, 30); // 高度可根据需要调整
    lv_obj_set_style_bg_color(spacer_btn, lv_color_black(), 0);
    lv_obj_clear_flag(spacer_btn, LV_OBJ_FLAG_CLICKABLE);           // 不可点击
    lv_obj_set_style_border_width(spacer_btn, 2, 0);                // 边框宽度2像素
    lv_obj_set_style_border_color(spacer_btn, lv_color_black(), 0); // 边框颜色为黑色
    lv_obj_set_style_border_opa(spacer_btn, LV_OPA_COVER, 0);       // 完全不透明

    lv_obj_t *btn;


    btn = lv_list_add_button(message_list, LV_SYMBOL_WIFI, "message1");
    lv_obj_t *icon1 = lv_obj_get_child(btn, 0); // 获取图标label
    if (icon1)
    {
        lv_obj_set_style_text_color(icon1, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
 
    }
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_50, 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);

    lv_obj_set_style_radius(btn, 10, 0); // 设置圆角
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);

    
    // 修改：在按钮内部创建容器
    lv_obj_t *btn_container1 = lv_obj_create(btn);
    lv_obj_remove_style_all(btn_container1); // 移除默认样式
    lv_obj_set_size(btn_container1, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(btn_container1, LV_FLEX_FLOW_COLUMN); // 垂直布局
    lv_obj_set_style_pad_all(btn_container1, 5, 0);            // 添加内边距

    // 创建标题容器（水平布局）
    lv_obj_t *title_container1 = lv_obj_create(btn_container1);
    lv_obj_set_scroll_dir(btn_container1, LV_DIR_NONE);
    lv_obj_remove_style_all(title_container1);
    lv_obj_set_size(title_container1, 400, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(title_container1, LV_FLEX_FLOW_ROW); // 水平布局
    lv_obj_set_style_pad_all(title_container1, 0, 0);
    lv_obj_set_style_bg_opa(title_container1, LV_OPA_TRANSP, 0);

    // 创建标题标签（message2）
    lv_obj_t *title_label1 = lv_label_create(title_container1);
    lv_label_set_text(title_label1, "Notification");
 
    lv_obj_set_style_text_align(title_label1, LV_TEXT_ALIGN_LEFT, 0);
    // lv_obj_set_width(title_label2, lv_pct(100));
    lv_obj_set_width(title_label1, 200);

    // 创建时间标签（右侧）
    lv_obj_t *time_label1 = lv_label_create(title_container1);
    lv_label_set_text(time_label1, "1 mins ago..");

    lv_obj_set_style_text_color(time_label1, lv_color_white(), 0);
    lv_obj_set_style_text_opa(time_label1, LV_OPA_20, 0); // 20%透明度
    lv_obj_set_style_text_align(time_label1, LV_TEXT_ALIGN_RIGHT, 0);

    // lv_obj_set_style_margin_bottom(title_label2, 5, 0);//标题和内容之间的间距
    lv_obj_set_style_margin_bottom(title_container1, 5, 0); // 标题和内容之间的间距

  

    // 创建内容标签（手机停机）
    lv_obj_t *content_label1 = lv_label_create(btn_container1);
    lv_label_set_text(content_label1, "Not find WiFi");
    lv_obj_set_style_text_color(content_label1, lv_color_white(), 0);
    lv_obj_set_style_text_opa(content_label1, LV_OPA_20, 0); // 20%透明度

    lv_obj_set_style_text_align(content_label1, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_width(content_label1, lv_pct(100));

    // 设置边框样式
    lv_obj_set_style_border_width(btn, 2, 0);                // 边框宽度2像素
    lv_obj_set_style_border_color(btn, lv_color_black(), 0); // 边框颜色为黑色
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, 0);       // 完全不透明

    lv_obj_set_width(btn, 400);
    lv_obj_set_height(btn, 100);
    lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_event_cb(btn, demo_message_event_handler, LV_EVENT_LONG_PRESSED, NULL); // LV_EVENT_CLICKED or LV_EVENT_LONG_PRESSED
    // lv_obj_add_event_cb(btn, message_show_event_handler, LV_EVENT_SHORT_CLICKED, imgtext1);
    // // 添加手势事件处理
    // lv_obj_add_event_cb(btn, gesture_handler, LV_EVENT_GESTURE, NULL);
    // lv_obj_add_flag(btn, LV_OBJ_FLAG_GESTURE_BUBBLE); // 允许手势事件冒泡

    lv_obj_set_style_margin_bottom(btn, 10, 0);

    btn = lv_list_add_button(message_list, LV_SYMBOL_BLUETOOTH, "message2");
    // static lv_style_t style_icon;
    // lv_style_init(&style_icon);
    // lv_style_set_text_color(&style_icon, lv_palette_main(LV_PALETTE_BLUE)); // 设为蓝色
    // lv_obj_add_style(LV_SYMBOL_BLUETOOTH, &style_icon, LV_PART_MAIN); // 应用样式
    lv_obj_t *icon2 = lv_obj_get_child(btn, 0); // 获取图标label
    if (icon2)
    {
        lv_obj_set_style_text_color(icon2, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    }

    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_50, 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);

    lv_obj_set_style_radius(btn, 10, 0); // 设置圆角
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);

    // 修改：在按钮内部创建容器
    lv_obj_t *btn_container2 = lv_obj_create(btn);
    lv_obj_remove_style_all(btn_container2); // 移除默认样式
    lv_obj_set_size(btn_container2, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(btn_container2, LV_FLEX_FLOW_COLUMN); // 垂直布局
    lv_obj_set_style_pad_all(btn_container2, 5, 0);            // 添加内边距

    // 创建标题容器（水平布局）
    lv_obj_t *title_container2 = lv_obj_create(btn_container2);
    lv_obj_set_scroll_dir(btn_container2, LV_DIR_NONE);

    lv_obj_remove_style_all(title_container2);
    lv_obj_set_size(title_container2, 400, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(title_container2, LV_FLEX_FLOW_ROW); // 水平布局
    lv_obj_set_style_pad_all(title_container2, 0, 0);
    lv_obj_set_style_bg_opa(title_container2, LV_OPA_TRANSP, 0);

    // 创建标题标签（message2）
    lv_obj_t *title_label2 = lv_label_create(title_container2);
    lv_label_set_text(title_label2, "Notification");

    lv_obj_set_style_text_align(title_label2, LV_TEXT_ALIGN_LEFT, 0);
    // lv_obj_set_width(title_label2, lv_pct(100));
    lv_obj_set_width(title_label2, 200);

    // 创建时间标签（右侧）
    lv_obj_t *time_label2 = lv_label_create(title_container2);
    lv_label_set_text(time_label2, "2 mins ago..");
   
    lv_obj_set_style_text_color(time_label2, lv_color_white(), 0);
    lv_obj_set_style_text_opa(time_label2, LV_OPA_20, 0); // 20%透明度
    lv_obj_set_style_text_align(time_label2, LV_TEXT_ALIGN_RIGHT, 0);

    // lv_obj_set_style_margin_bottom(title_label2, 5, 0);//标题和内容之间的间距
    lv_obj_set_style_margin_bottom(title_container2, 5, 0); // 标题和内容之间的间距

    // 创建内容标签（手机停机）
    lv_obj_t *content_label2 = lv_label_create(btn_container2);
    lv_label_set_text(content_label2, "Not find Bluetooth device");
    lv_obj_set_style_text_color(content_label2, lv_color_white(), 0);
    lv_obj_set_style_text_opa(content_label2, LV_OPA_20, 0); // 20%透明度

    lv_obj_set_style_text_align(content_label2, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_width(content_label2, lv_pct(100));

   
    // 设置边框样式
    lv_obj_set_style_border_width(btn, 2, 0);                // 边框宽度2像素
    lv_obj_set_style_border_color(btn, lv_color_black(), 0); // 边框颜色为黑色
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, 0);       // 完全不透明

    lv_obj_set_width(btn, 400);
    lv_obj_set_height(btn, 100);
    // lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_event_cb(btn, demo_message_event_handler, LV_EVENT_LONG_PRESSED, NULL); // LV_EVENT_CLICKED or LV_EVENT_LONG_PRESSED
    // lv_obj_add_event_cb(btn, message_show_event_handler, LV_EVENT_SHORT_CLICKED, imgtext2);

    lv_obj_set_style_margin_bottom(btn, 10, 0);

    btn = lv_list_add_button(message_list, LV_SYMBOL_BLUETOOTH, "message3");

    

    lv_obj_t *icon3 = lv_obj_get_child(btn, 0); // 获取图标label
    if (icon3)
    {
        lv_obj_set_style_text_color(icon3, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    }

    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_50, 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);

    lv_obj_set_style_radius(btn, 10, 0); // 设置圆角
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);

   
    // 修改：在按钮内部创建容器
    lv_obj_t *btn_container3 = lv_obj_create(btn);
    lv_obj_remove_style_all(btn_container3); // 移除默认样式
    lv_obj_set_size(btn_container3, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(btn_container3, LV_FLEX_FLOW_COLUMN); // 垂直布局
    lv_obj_set_style_pad_all(btn_container3, 5, 0);            // 添加内边距

    // 创建标题容器（水平布局）
    lv_obj_t *title_container3 = lv_obj_create(btn_container3);
    lv_obj_set_scroll_dir(btn_container3, LV_DIR_NONE);

    lv_obj_remove_style_all(title_container3);
    lv_obj_set_size(title_container3, 400, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(title_container3, LV_FLEX_FLOW_ROW); // 水平布局
    lv_obj_set_style_pad_all(title_container3, 0, 0);
    lv_obj_set_style_bg_opa(title_container3, LV_OPA_TRANSP, 0);

    // 创建标题标签（message2）
    lv_obj_t *title_label3 = lv_label_create(title_container3);
    lv_label_set_text(title_label3, "Notification");
 
    lv_obj_set_style_text_align(title_label3, LV_TEXT_ALIGN_LEFT, 0);
    // lv_obj_set_width(title_label2, lv_pct(100));
    lv_obj_set_width(title_label3, 200);

    // 创建时间标签（右侧）
    lv_obj_t *time_label3 = lv_label_create(title_container3);
    lv_label_set_text(time_label3, "3 mins ago..");

    lv_obj_set_style_text_color(time_label3, lv_color_white(), 0);
    lv_obj_set_style_text_opa(time_label3, LV_OPA_20, 0); // 20%透明度
    lv_obj_set_style_text_align(time_label3, LV_TEXT_ALIGN_RIGHT, 0);

    // lv_obj_set_style_margin_bottom(title_label2, 5, 0);//标题和内容之间的间距
    lv_obj_set_style_margin_bottom(title_container3, 5, 0); // 标题和内容之间的间距

    // 创建内容标签（手机停机）
    lv_obj_t *content_label3 = lv_label_create(btn_container3);
    lv_label_set_text(content_label3, "found Bluetooth device");
    lv_obj_set_style_text_color(content_label3, lv_color_white(), 0);
    lv_obj_set_style_text_opa(content_label3, LV_OPA_20, 0); // 20%透明度

    lv_obj_set_style_text_align(content_label3, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_width(content_label3, lv_pct(100));

    

    lv_obj_set_style_border_width(btn, 2, 0);                // 边框宽度2像素
    lv_obj_set_style_border_color(btn, lv_color_black(), 0); // 边框颜色为黑色
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, 0);       // 完全不透明
    lv_obj_set_width(btn, 400);
    lv_obj_set_height(btn, 100);
    lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_event_cb(btn, demo_message_event_handler, LV_EVENT_LONG_PRESSED, NULL); // LV_EVENT_CLICKED or LV_EVENT_LONG_PRESSED
    // lv_obj_add_event_cb(btn, message_show_event_handler, LV_EVENT_SHORT_CLICKED, imgtext3);

    lv_obj_set_style_margin_bottom(btn, 10, 0);

    btn = lv_list_add_button(message_list, LV_SYMBOL_WIFI, "message4");
    lv_obj_t *icon4 = lv_obj_get_child(btn, 0); // 获取图标label
    if (icon4)
    {
        lv_obj_set_style_text_color(icon4, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    }
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_50, 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
 
    lv_obj_set_style_radius(btn, 10, 0); // 设置圆角
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);

    
    // 修改：在按钮内部创建容器
    lv_obj_t *btn_container4 = lv_obj_create(btn);
    lv_obj_remove_style_all(btn_container4); // 移除默认样式
    lv_obj_set_size(btn_container4, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(btn_container4, LV_FLEX_FLOW_COLUMN); // 垂直布局
    lv_obj_set_style_pad_all(btn_container4, 5, 0);            // 添加内边距

    // 创建标题容器（水平布局）
    lv_obj_t *title_container4 = lv_obj_create(btn_container4);
    lv_obj_set_scroll_dir(btn_container4, LV_DIR_NONE);

    lv_obj_remove_style_all(title_container4);
    lv_obj_set_size(title_container4, 400, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(title_container4, LV_FLEX_FLOW_ROW); // 水平布局
    lv_obj_set_style_pad_all(title_container4, 0, 0);
    lv_obj_set_style_bg_opa(title_container4, LV_OPA_TRANSP, 0);

    // 创建标题标签（message2）
    lv_obj_t *title_label4 = lv_label_create(title_container4);
    lv_label_set_text(title_label4, "Notification");

    lv_obj_set_style_text_align(title_label4, LV_TEXT_ALIGN_LEFT, 0);
    // lv_obj_set_width(title_label2, lv_pct(100));
    lv_obj_set_width(title_label4, 200);

    // 创建时间标签（右侧）
    lv_obj_t *time_label4 = lv_label_create(title_container4);
    lv_label_set_text(time_label4, "4 mins ago..");

    lv_obj_set_style_text_color(time_label4, lv_color_white(), 0);
    lv_obj_set_style_text_opa(time_label4, LV_OPA_20, 0); // 20%透明度
    lv_obj_set_style_text_align(time_label4, LV_TEXT_ALIGN_RIGHT, 0);

    // lv_obj_set_style_margin_bottom(title_label2, 5, 0);//标题和内容之间的间距
    lv_obj_set_style_margin_bottom(title_container4, 5, 0); // 标题和内容之间的间距



    // 创建内容标签（手机停机）
    lv_obj_t *content_label4 = lv_label_create(btn_container4);
    lv_label_set_text(content_label4, "found WiFi");
    lv_obj_set_style_text_color(content_label4, lv_color_white(), 0);
    lv_obj_set_style_text_opa(content_label4, LV_OPA_20, 0); // 20%透明度
 
    lv_obj_set_style_text_align(content_label4, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_width(content_label4, lv_pct(100));
   
    lv_obj_set_style_border_width(btn, 2, 0);                // 边框宽度2像素
    lv_obj_set_style_border_color(btn, lv_color_black(), 0); // 边框颜色为黑色
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, 0);       // 完全不透明
    lv_obj_set_width(btn, 400);
    lv_obj_set_height(btn, 100);
    lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_event_cb(btn, demo_message_event_handler, LV_EVENT_LONG_PRESSED, NULL);
    // LV_EVENT_CLICKED or LV_EVENT_LONG_PRESSED
    // lv_obj_add_event_cb(btn, message_show_event_handler, LV_EVENT_SHORT_CLICKED, imgtext4);

    // 下行是对按钮状态过渡效果的设置，LV_STYLE_TRANSFORM_WIDTH表示宽度变化，LV_STYLE_IMAGE_RECOLOR_OPA表示颜色透明度变化，以0结尾作为结束
    //  对消息进行动画效果设置，常按高度变化
}
void draw_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_DIR_TOP)
    {
        // 删除主页面
        if (main_page)
        {
            lv_obj_del(main_page);
            main_page = NULL;
        }
        // lv_dir_t dir = lv_indev_get_gesture_dir(lv_event_get_indev(e));
        init_message_list_page(messageCenterScreen);
        // lv_obj_t * mbox = lv_obj_get_parent(lv_event_get_target(e));
        // lv_obj_del(mbox);

        // lv_obj_del(lv_event_get_target(e)); // 删除当前页面
    }
}
void watch_message_list_init(void)
{
    main_page = lv_obj_create(lv_screen_active());
    lv_obj_set_size(main_page, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    static lv_style_t circle; // 圆形的通用界面风格
    lv_style_init(&circle);
    lv_style_set_radius(&circle, 228); // 455的一半 227，保持圆形
    // lv_style_set_bg_color(&circle, lv_palette_main(LV_PALETTE_GREEN));
    lv_obj_add_style(main_page, &circle, 0);

    lv_obj_center(main_page);
}

void message_center_start(lv_obj_t *messageCenterScreen)
{
    LV_LOG_USER("message_center_start in");
    lv_obj_set_size(messageCenterScreen, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_set_style_bg_color(messageCenterScreen, lv_color_hex(0x030301), LV_PART_MAIN);
    lv_obj_align(messageCenterScreen, LV_ALIGN_CENTER, 0, -LV_CIRCLE_WATCH);
    lv_obj_set_style_radius(messageCenterScreen, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_pad_all(messageCenterScreen, 0, 0);
    lv_obj_set_style_border_width(messageCenterScreen, 0, 0);
    lv_obj_set_style_outline_width(messageCenterScreen, 0, 0);

    if (messageCenterObj == NULL)
    {
        messageCenterObj = malloc(sizeof(struct MESSAGE_CENTER));
    }
    // init_message_list_page(messageCenterScreen);
    init_message_list_page_bak(messageCenterScreen); 
}

#endif