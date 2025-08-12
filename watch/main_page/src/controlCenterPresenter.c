#include <lvgl/lvgl.h>

#include "controlCenterScreen.h"
// #include "../../frameworks/runtimes/services/brightness/include/BrightnessServiceC.h"


void flight_handle(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        int index = (int)e->user_data;
        centrolCenterObj->item[index].status = !centrolCenterObj->item[index].status;
        item_status_ctrl(index, item_handle[index].item_var_on(), item_handle[index].item_var_off());
    }

    break;
    case LV_EVENT_PRESSING:
    {
    }

    break;

    default:
        break;
    }
}

void setting_handle(lv_event_t *e)
{
    int index = (int)e->user_data;
    centrolCenterObj->item[index].status = !centrolCenterObj->item[index].status;
    item_status_ctrl(index, item_handle[index].item_var_on(), item_handle[index].item_var_off());
    // item_status_ctrl(index, &setting_on, &setting_off);
}

void _4g_handle(lv_event_t *e)
{
    int index = (int)e->user_data;
    centrolCenterObj->item[index].status = !centrolCenterObj->item[index].status;
    item_status_ctrl(index, item_handle[index].item_var_on(), item_handle[index].item_var_off());
}

void sound_handle(lv_event_t *e)
{
    int index = (int)e->user_data;
    centrolCenterObj->item[index].status = !centrolCenterObj->item[index].status;
    item_status_ctrl(index, item_handle[index].item_var_on(), item_handle[index].item_var_off());
}

void wifi_handle(lv_event_t *e)
{
    int index = (int)e->user_data;
    centrolCenterObj->item[index].status = !centrolCenterObj->item[index].status;
    item_status_ctrl(index, item_handle[index].item_var_on(), item_handle[index].item_var_off());
}

void bluebooth_handle(lv_event_t *e)
{
    int index = (int)e->user_data;
    centrolCenterObj->item[index].status = !centrolCenterObj->item[index].status;
    item_status_ctrl(index, item_handle[index].item_var_on(), item_handle[index].item_var_off());
}

bool setItemStatus(int index, bool status)
{
    centrolCenterObj->item[index].status = status;
    return true;
}

bool getItemStatus(int index)
{
    return centrolCenterObj->item[index].status;
}

void brightness_slider_handle(lv_event_t *e)
{
    // lv_obj_t *target = lv_event_get_target(e);
    // int32_t curBrightness = lv_slider_get_value(target);

    //   BrightnessService_setTargetBrightness((curBrightness/brightnessValue.maxBrightness*255),BRIGHTNESS_RAMP_SPEED_DEFAULT);
    // BrightnessService_setTargetBrightness(100,BRIGHTNESS_RAMP_SPEED_DEFAULT);
}
