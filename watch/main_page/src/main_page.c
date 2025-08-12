
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "choose_dial.h"
#include "controlCenterScreen.h"
#include "../../message/include/message.h"
#include "../../launcher/include/app_list.h"
#include "../include/main_page.h"

MainPageData *mpd = NULL;
void dele_main_page(MainPageData *mpd)
{
    if (mpd->timer != 0)
    {
        lv_timer_delete(mpd->timer);
    }
    // 对同一对象多次监听必须要移除user_data，否则重新监听，user_data还是上次监听注册的user_data，会导致crash
    lv_obj_remove_event_cb_with_user_data(lv_screen_active(), sys_gesture_event_cb, mpd);
    lv_obj_delete(mpd->watchScr);
    free(mpd);
}

void update_time(lv_timer_t *timer)
{
    // LV_LOG_USER("update_time in");

    // not sure????????  MainPageData *mpd = timer->user_data;
    //  lv_mutex_lock(&mpd->lock);

    time_t t = time(NULL);
    struct tm *localTime = localtime(&t);
    // LV_LOG_USER("year %d, mon %d, day %d, hour %d, min %d, sec %d\r\n",
    //             localTime->tm_year+1900, localTime->tm_mon+1, localTime->tm_mday,
    //             localTime->tm_hour + 8, localTime->tm_min, localTime->tm_sec);
    uint16_t hour = (localTime->tm_hour + 8) % 24;
    uint16_t minute = localTime->tm_min;
    uint16_t second = localTime->tm_sec;

    if (!strcmp(mpd->tagBuf, "clock_dial"))
    {
        //old code
        // int second_w = lv_obj_get_width(mpd->secondObj);
        // int second_h = lv_obj_get_height(mpd->secondObj);
        // lv_obj_set_pos(mpd->secondObj, 0, -LV_CIRCLE_WATCH / 2);
        // lv_image_set_pivot(mpd->secondObj, second_w / 2, second_h);
        // lv_image_set_zoom(mpd->secondObj, 128);
        // lv_image_set_rotation(mpd->secondObj, second * 60);

        // int minute_w = lv_obj_get_width(mpd->minuteObj);
        // int minute_h = lv_obj_get_height(mpd->minuteObj);
        // uint16_t minuteRota = second + minute * 60;
        // lv_obj_set_pos(mpd->minuteObj, 0, -LV_CIRCLE_WATCH / 2);
        // lv_image_set_pivot(mpd->minuteObj, minute_w / 2, minute_h);
        // lv_image_set_zoom(mpd->minuteObj, 128);
        // lv_image_set_rotation(mpd->minuteObj, minuteRota);

        // int hour_w = lv_obj_get_width(mpd->hourObj);
        // int hour_h = lv_obj_get_height(mpd->hourObj);
        // uint16_t hourRota = minute * 5 + hour * 300;
        // lv_obj_set_pos(mpd->hourObj, 0, -LV_CIRCLE_WATCH / 2);
        // lv_image_set_pivot(mpd->hourObj, hour_w / 2, hour_h);
        // lv_image_set_zoom(mpd->hourObj, 64);
        // lv_image_set_rotation(mpd->hourObj, hourRota);

        //old code 

        LV_LOG_USER("There should be clock update time logic ,but we remove the relative image \n");
    }
    if (!strcmp(mpd->tagBuf, "digital_dial"))
    {
        char hourBuf[3] = {0};
        char minuteBuf[3] = {0};
        if (hour / 10)
        {
            sprintf(hourBuf, "%d", hour);
        }
        else
        {
            sprintf(hourBuf, "0%d", hour);
        }
        if (minute / 10)
        {
            sprintf(minuteBuf, "%d", minute);
        }
        else
        {
            sprintf(minuteBuf, "0%d", minute);
        }
        lv_label_set_text_fmt(mpd->hourObj, "%s : %s", hourBuf, minuteBuf);
    }
    // lv_mutex_unlock(&mpd->lock);
}



void sys_gesture_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    LV_LOG_USER("mpd2222222 = %p", mpd);

    switch (code)
    {
    case LV_EVENT_GESTURE:
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        if (dir == LV_DIR_BOTTOM)
        {
            if (!strcmp(mpd->current_page, HOME_PAGE))
            {
                mpd->cntCenter = lv_obj_create(mpd->watchScr);
                control_center_start(mpd->cntCenter);
                lv_obj_set_style_translate_y(mpd->cntCenter, lv_obj_get_height(mpd->watchScr), 0);
                strcpy(mpd->current_page, CONTER_PAGE);
            }
            else
            {
                bool is_msg_page = !strcmp(mpd->current_page, MESSAGE_PAGE);
                LV_LOG_USER("LV_DIR_BOTTOM + currentPage: %s, is_msg_page %d \n", mpd->current_page, is_msg_page);
                if (is_msg_page)
                {
                    LV_LOG_USER("LV_DIR_BOTTOM : %d, %d \n", (mpd != NULL), (mpd->msgCenter != NULL));
                    if (mpd != NULL && mpd->msgCenter != NULL) {
                        lv_obj_set_style_translate_y(mpd->msgCenter, 0, 0);
                        lv_obj_delete(mpd->msgCenter);
                        // free(mpd->msgCenter);
                        mpd->msgCenter = NULL;
                    }
                    // avoid wild pointer
                    strcpy(mpd->current_page, HOME_PAGE);
                }
                LV_LOG_USER("LV_DIR_BOTTOM - currentPage: %s, is_msg_page %d \n", mpd->current_page, is_msg_page);
            }
        }
        if (dir == LV_DIR_TOP)
        {
            if (!strcmp(mpd->current_page, HOME_PAGE))
            {
                LV_LOG_USER("Have Messgae");//用户日志查看
                mpd->msgCenter = lv_obj_create(mpd->watchScr);
                message_center_start(mpd->msgCenter);
                lv_obj_set_style_translate_y(mpd->msgCenter, lv_obj_get_height(mpd->watchScr), 0);
                strcpy(mpd->current_page, MESSAGE_PAGE);
            }
            else
            {
                bool is_cnt_page = !strcmp(mpd->current_page, CONTER_PAGE);
                if (is_cnt_page)
                {
                    lv_obj_set_style_translate_y(mpd->cntCenter, 0, 0);
                    control_center_stop();
                    lv_obj_delete(mpd->cntCenter);
                    centrolCenterObj = NULL;
                    mpd->cntCenter = NULL;
                    // avoid wild pointer
                    strcpy(mpd->current_page, HOME_PAGE);
                }
            }
        }
        if (dir == LV_DIR_LEFT)
        {
            if (!strcmp(mpd->current_page, HOME_PAGE))
            {
                mpd->appListCenter = lv_obj_create(mpd->watchScr);
                apps_center_start(mpd->appListCenter);
                lv_obj_set_style_translate_y(mpd->appListCenter, lv_obj_get_height(mpd->watchScr), 0);
                strcpy(mpd->current_page, APPS_PAGE);
            }
            else
            {
                bool is_home_page = !strcmp(mpd->current_page, HOME_PAGE);
                LV_LOG_USER("LV_DIR_LEFT + currentPage: %s, is_home_page %d \n", mpd->current_page, is_home_page);
            }
        }
        if (dir == LV_DIR_RIGHT)
        {
            
            if (!strcmp(mpd->current_page, HOME_PAGE))
            {
                LV_LOG_USER("LV_DIR_RIGHT currentPage: %s  \n", mpd->current_page);
            }
            else
            {

                // actrually all LV_DIR_RIGHT event can return to home_page,pull out
                LV_LOG_USER("Before LV_DIR_RIGHT currentPage: %s  \n", mpd->current_page);
                // correct print should be other page except homepage
                if (mpd->appListCenter != NULL)
                {
                    lv_obj_set_style_translate_y(mpd->appListCenter, 0, 0);
                    lv_obj_delete(mpd->appListCenter);
                   // free(mpd->appListCenter);
                    mpd->appListCenter = NULL;
                    // avoid wild pointer
                    // lv_obj_set_style_translate_y(mpd->mainPage, LV_CIRCLE_WATCH, 0);
                }
                if (mpd->msgCenter != NULL){
                    lv_obj_set_style_translate_y(mpd->msgCenter, 0, 0);
                    lv_obj_delete(mpd->msgCenter);
                    //free(mpd->msgCenter);
                    // avoid wild pointer
                    mpd->msgCenter = NULL;  
                }
                    
                if (mpd->cntCenter != NULL){
                    lv_obj_set_style_translate_y(mpd->cntCenter, 0, 0);
                    lv_obj_delete(mpd->cntCenter);
                    //free(mpd->cntCenter);
                    mpd->cntCenter = NULL;
                }
                // avoid wild pointer

                strcpy(mpd->current_page, HOME_PAGE);
                LV_LOG_USER(" After LV_DIR_RIGHT  currentPage: %s \n", mpd->current_page);
            }
        }
        break;
    default:
        break;
    }
}

void create_clock_dial()
{
    strcpy(mpd->current_page, HOME_PAGE);
    LV_LOG_USER("create_clock_dial in");
    mpd->watchScr = create_watch_scr();
    mpd->mainPage = lv_obj_create(mpd->watchScr); // new code 
    // LV_IMAGE_DECLARE(clock_dial);

    lv_obj_set_size(mpd->mainPage, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_center(mpd->mainPage);

    // new code
    lv_obj_t *clockLabel = lv_label_create(mpd->mainPage);
    lv_label_set_text(clockLabel, "Simulate ClockDial");
    lv_obj_set_style_text_color(clockLabel, lv_color_black(), 0);
    lv_obj_align_to(clockLabel, mpd->mainPage, LV_ALIGN_CENTER, 0, 0); 
    //new code



    strcpy(mpd->tagBuf, "clock_dial");

    lv_obj_add_flag(mpd->mainPage, LV_OBJ_FLAG_CLICKABLE);

}

void create_digital_dial()
{
    LV_LOG_USER("create_digital_dial in");
    strcpy(mpd->current_page, HOME_PAGE);
    mpd->watchScr = create_watch_scr();
    mpd->mainPage = lv_obj_create(mpd->watchScr); // new code and error
    // LV_IMAGE_DECLARE(digital_dial);

    lv_obj_set_size(mpd->mainPage, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_center(mpd->mainPage);

    mpd->hourObj = lv_label_create(mpd->mainPage);


    lv_obj_set_style_text_color(mpd->hourObj, lv_color_black(), 0);
    lv_obj_center(mpd->hourObj);

    mpd->timer = lv_timer_create(update_time, 1000, mpd);
    lv_timer_ready(mpd->timer);

    strcpy(mpd->tagBuf, "digital_dial");

    lv_obj_add_flag(mpd->mainPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(mpd->mainPage, LV_OBJ_FLAG_SCROLLABLE);
   // lv_obj_add_event_cb(mpd->mainPage, long_pressed_event_cb, LV_EVENT_LONG_PRESSED, mpd);
}

void main_page(const char *str)
{
    LV_LOG_USER("main_page in-----");
    mpd = (MainPageData *)malloc(sizeof(MainPageData));
    memset(mpd, 0, sizeof(MainPageData));
    // lv_mutex_init(mpd->lock);
    // LV_LOG_USER("000000000 [%p]", mpd);

    strcpy(mpd->current_page, "");

    // lv_mutex_lock(mpd->lock);

    if (!strcmp(str, "digital_dial"))
    {
        create_digital_dial();
    }

    lv_obj_add_event_cb(lv_screen_active(), sys_gesture_event_cb, LV_EVENT_GESTURE, mpd);
    
    LV_LOG_USER("mpd1111111 = %p", mpd);
    // lv_mutex_unlock(mpd->lock);
}