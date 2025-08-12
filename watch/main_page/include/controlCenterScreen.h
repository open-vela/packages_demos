#ifndef CENTER_COTROL_SCREEN_H
#define CENTER_COTROL_SCREEN_H
#include <lvgl/lvgl.h>
#include "controlCenterPresenter.h"

// screen size
#define LV_CIRCLE_WATCH 455
#define CURRENT_SIZE_ZOOM (LV_CIRCLE_WATCH / 455)

enum ITEM_IMAGE
{
    ITEM_FLIGHT,
    ITEM_SETTING,
    ITEM_4G,
    ITEM_SOUND,
    ITEM_WIFI,
    ITEM_BLUETOOTH,
    ITEM_IMAGE_COUNT,
};

struct ITEM
{
    bool status;
    lv_obj_t *itemObj;
    enum ITEM_IMAGE image;
};

struct ITEM_HANDLE
{
    enum ITEM_IMAGE image;
    lv_event_cb_t (*item_cb)(lv_event_t *e);
    lv_image_dsc_t *(*item_var_on)(void);
    lv_image_dsc_t *(*item_var_off)(void);
};

struct CENTROL_CENTER
{
    lv_style_t borderStyle;
    lv_obj_t *statusBar;
    lv_obj_t *phoneConnectObj;
    lv_obj_t *gpsObj;
    lv_obj_t *signalStrengthObj;
    lv_obj_t *wifiStrengthObj;
    lv_obj_t *middleObj;
    lv_obj_t *batteryObj;
    lv_obj_t *batteryLabel;
    lv_obj_t *batteryImage;
    lv_obj_t *brightnessObj;
    lv_obj_t *brightnessSlider;
    lv_obj_t *brightnessLow;
    lv_obj_t *brightnessHigh;
    struct ITEM item[ITEM_IMAGE_COUNT];
};

struct BRIGHTNESS_VALUE
{
    int32_t minBrightness;
    int32_t maxBrightness;
};

extern const struct ITEM_HANDLE item_handle[];
extern struct CENTROL_CENTER *centrolCenterObj;
extern struct BRIGHTNESS_VALUE brightnessValue;
void item_status_ctrl(enum ITEM_IMAGE index, lv_image_dsc_t *image_on, lv_image_dsc_t *image_off);
void control_center_start(lv_obj_t* controlCenterScreen);

#endif