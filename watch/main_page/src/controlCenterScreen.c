#include "controlCenterScreen.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

// status sar size
#define LV_STATUS_BAR_WIDTH (300 * CURRENT_SIZE_ZOOM)
#define LV_STATUS_BAR_HEIGHT (70 * CURRENT_SIZE_ZOOM)
// control center  middle centent size
#define LV_CONTROL_CENTER_MIDDLE_WIDTH (360 * CURRENT_SIZE_ZOOM)
#define LV_CONTROL_CENTER_MIDDLE_HEIGHT (240 * CURRENT_SIZE_ZOOM)
// control center  battery size
#define LV_CONTROL_BATTERY_WIDTH (210 * CURRENT_SIZE_ZOOM)
#define LV_CONTROL_BATTERY_HEIGHT (60 * CURRENT_SIZE_ZOOM)
// control center  brightness obj size
#define LV_CONTROL_BRIGHTNESS_OBJ_WIDTH (360 * CURRENT_SIZE_ZOOM)
#define LV_CONTROL_BRIGHTNESS_OBJ_HEIGHT (80 * CURRENT_SIZE_ZOOM)
// control center  brightness slider size
#define LV_CONTROL_BRIGHTNESS_SLIDER_WIDTH (250 * CURRENT_SIZE_ZOOM)
#define LV_CONTROL_BRIGHTNESS_SLIDER_HEIGHT (18 * CURRENT_SIZE_ZOOM)
// grid size
#define LV_GRID_SIZE (100 * CURRENT_SIZE_ZOOM)
// grid position
static lv_coord_t col_dsc[] = {LV_GRID_SIZE, LV_GRID_SIZE, LV_GRID_SIZE, LV_GRID_TEMPLATE_LAST};
static lv_coord_t row_dsc[] = {LV_GRID_SIZE, LV_GRID_SIZE, LV_GRID_TEMPLATE_LAST};

#define GRID_COL_NUM (sizeof(col_dsc) / sizeof(lv_coord_t) - 1)
#define GRID_ROW_NUM (sizeof(row_dsc) / sizeof(lv_coord_t) - 1)

const struct ITEM_HANDLE item_handle[ITEM_IMAGE_COUNT] =
    {
        {ITEM_FLIGHT, flight_handle},
        {ITEM_SETTING, setting_handle},
        {ITEM_4G, _4g_handle},
        {ITEM_SOUND, sound_handle},
        {ITEM_BLUETOOTH, wifi_handle},
        {ITEM_IMAGE_COUNT, bluebooth_handle},
};

lv_obj_t *controlCenterScreen = NULL; // 控制中心的Screen
struct CENTROL_CENTER *centrolCenterObj = NULL;
struct BRIGHTNESS_VALUE brightnessValue;

// 1. 定义symbol数组
static const char *item_symbols[ITEM_IMAGE_COUNT] = {
    LV_SYMBOL_CALL,     // phone
    LV_SYMBOL_SETTINGS, // 设置
    LV_SYMBOL_USB,      // 4G
    LV_SYMBOL_AUDIO,    // 声音
    LV_SYMBOL_WIFI,     // WIFI
    LV_SYMBOL_BLUETOOTH // 蓝牙
};

// 2. 修改item_status_ctrl，设置symbol和颜色
void item_status_ctrl(enum ITEM_IMAGE index, lv_image_dsc_t *image_on, lv_image_dsc_t *image_off)
{
    lv_obj_t *label = centrolCenterObj->item[index].itemObj;
    lv_label_set_text(label, item_symbols[index]);
   
}

static void status_bar_create(lv_obj_t *screen)
{
  
    centrolCenterObj->statusBar = lv_obj_create(screen);
    lv_obj_set_size(centrolCenterObj->statusBar, LV_CIRCLE_WATCH, 70);
    lv_obj_clear_flag(centrolCenterObj->statusBar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_radius(centrolCenterObj->statusBar, LV_CIRCLE_WATCH, LV_PART_MAIN); // new code
    lv_obj_set_style_clip_corner(centrolCenterObj->statusBar, true, LV_PART_MAIN);       // new code
    lv_obj_set_style_border_width(centrolCenterObj->statusBar, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(centrolCenterObj->statusBar, lv_color_hex(0x3A7BD5), LV_PART_MAIN);
    lv_obj_align_to(centrolCenterObj->statusBar, screen, LV_ALIGN_TOP_MID, 0, -18);

    // centrolCenterObj->phoneConnectObj = lv_image_create(centrolCenterObj->statusBar);
  
    centrolCenterObj->phoneConnectObj = lv_label_create(centrolCenterObj->statusBar);
    lv_label_set_text(centrolCenterObj->phoneConnectObj, LV_SYMBOL_CALL);
    lv_obj_set_style_text_color(centrolCenterObj->phoneConnectObj, lv_color_white(), 0);
    lv_obj_align_to(centrolCenterObj->phoneConnectObj, centrolCenterObj->statusBar, LV_ALIGN_CENTER, -80, 0);

    // centrolCenterObj->signalStrengthObj = lv_image_create(centrolCenterObj->statusBar);
 
    centrolCenterObj->signalStrengthObj = lv_label_create(centrolCenterObj->statusBar);
    lv_label_set_text(centrolCenterObj->signalStrengthObj, LV_SYMBOL_USB);
  
    lv_obj_set_style_text_color(centrolCenterObj->signalStrengthObj, lv_color_white(), 0);
    lv_obj_align_to(centrolCenterObj->signalStrengthObj, centrolCenterObj->statusBar, LV_ALIGN_CENTER, -30, 0);

    // centrolCenterObj->wifiStrengthObj = lv_image_create(centrolCenterObj->statusBar);
 
    centrolCenterObj->wifiStrengthObj = lv_label_create(centrolCenterObj->statusBar);
    lv_label_set_text(centrolCenterObj->wifiStrengthObj, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(centrolCenterObj->wifiStrengthObj, lv_color_white(), 0);
    lv_obj_align_to(centrolCenterObj->wifiStrengthObj, centrolCenterObj->statusBar, LV_ALIGN_CENTER, 25, 0);

    // centrolCenterObj->gpsObj = lv_image_create(centrolCenterObj->statusBar);
 
    centrolCenterObj->gpsObj = lv_label_create(centrolCenterObj->statusBar);
    lv_label_set_text(centrolCenterObj->gpsObj, LV_SYMBOL_GPS);
    lv_obj_set_style_text_color(centrolCenterObj->gpsObj, lv_color_white(), 0);
    lv_obj_align_to(centrolCenterObj->gpsObj, centrolCenterObj->statusBar, LV_ALIGN_CENTER, 75, 0);
}

// static void drag_event_handler(lv_event_t *e)

static void middle_content_create(lv_obj_t *screen)
{
    centrolCenterObj->middleObj = lv_obj_create(screen);
    lv_obj_set_size(centrolCenterObj->middleObj, LV_CONTROL_CENTER_MIDDLE_WIDTH, LV_CONTROL_CENTER_MIDDLE_HEIGHT);
    lv_obj_set_style_bg_color(centrolCenterObj->middleObj, lv_color_hex(0x030301), LV_PART_MAIN);
    lv_obj_align_to(centrolCenterObj->middleObj, screen, LV_ALIGN_CENTER, 0, -30);
    lv_obj_add_style(centrolCenterObj->middleObj, &centrolCenterObj->borderStyle, 0);
    lv_obj_set_scrollbar_mode(centrolCenterObj->middleObj, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_grid_column_dsc_array(centrolCenterObj->middleObj, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(centrolCenterObj->middleObj, row_dsc, 0);
    lv_obj_set_layout(centrolCenterObj->middleObj, LV_LAYOUT_GRID);

    for (int i = 0; i < ITEM_IMAGE_COUNT; i++)
    {
        // centrolCenterObj->item[i].itemObj = lv_image_create(centrolCenterObj->middleObj);
        centrolCenterObj->item[i].itemObj = lv_label_create(centrolCenterObj->middleObj);
        //centrolCenterObj->item[i].image = i;
       // centrolCenterObj->item[i].status = getItemStatus(i);
        item_status_ctrl(i, NULL, NULL); // 只用symbol，不用图片
        lv_obj_set_style_text_color(centrolCenterObj->item[i].itemObj, lv_color_white(), 0); // 未选中白色
       // lv_obj_add_flag(centrolCenterObj->item[i].itemObj, LV_OBJ_FLAG_CLICKABLE);
        //lv_obj_add_event_cb(centrolCenterObj->item[i].itemObj, item_handle[i].item_cb, LV_EVENT_CLICKED | LV_EVENT_PRESSING, i);
        lv_obj_set_grid_cell(centrolCenterObj->item[i].itemObj, LV_GRID_ALIGN_CENTER, (i % GRID_COL_NUM), 1, LV_GRID_ALIGN_CENTER, (i / GRID_COL_NUM), 1);
    }
}

static void battery_create(lv_obj_t *screen)
{
    centrolCenterObj->batteryObj = lv_obj_create(screen);
    lv_obj_set_scrollbar_mode(centrolCenterObj->batteryObj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(centrolCenterObj->batteryObj, LV_CONTROL_BATTERY_WIDTH, LV_CONTROL_BATTERY_HEIGHT);
    lv_obj_set_style_bg_color(centrolCenterObj->batteryObj, lv_color_hex(0x030301), LV_PART_MAIN);
    lv_obj_add_style(centrolCenterObj->batteryObj, &centrolCenterObj->borderStyle, 0);
    lv_obj_align_to(centrolCenterObj->batteryObj, screen, LV_ALIGN_BOTTOM_MID, 0, 0);

    centrolCenterObj->batteryLabel = lv_label_create(centrolCenterObj->batteryObj);
    lv_label_set_text(centrolCenterObj->batteryLabel, "75%");
    lv_obj_set_style_text_color(centrolCenterObj->batteryLabel, lv_color_hex(0x33FFFF), 0);
   
    lv_obj_add_style(centrolCenterObj->batteryLabel, &centrolCenterObj->borderStyle, 0);
    lv_obj_align_to(centrolCenterObj->batteryLabel, centrolCenterObj->batteryObj, LV_ALIGN_CENTER, 25, 0);

    // centrolCenterObj->batteryImage = lv_image_create(centrolCenterObj->batteryObj);

    centrolCenterObj->batteryImage = lv_label_create(centrolCenterObj->batteryObj);
    lv_label_set_text(centrolCenterObj->batteryImage, LV_SYMBOL_BATTERY_3);
    lv_obj_clear_flag(centrolCenterObj->batteryObj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_text_color(centrolCenterObj->batteryImage, lv_color_white(), 0);
    lv_obj_align_to(centrolCenterObj->batteryImage, centrolCenterObj->batteryLabel, LV_ALIGN_OUT_LEFT_MID, -10, 0);
}

static void
brightness_create(lv_obj_t *screen)
{
    centrolCenterObj->brightnessObj = lv_obj_create(screen);
    lv_obj_set_size(centrolCenterObj->brightnessObj, LV_CONTROL_BRIGHTNESS_OBJ_WIDTH, LV_CONTROL_BRIGHTNESS_OBJ_HEIGHT);
    lv_obj_set_style_bg_color(centrolCenterObj->brightnessObj, lv_color_hex(0x030301), LV_PART_MAIN);
    lv_obj_add_style(centrolCenterObj->brightnessObj, &centrolCenterObj->borderStyle, 0);
    lv_obj_set_scrollbar_mode(centrolCenterObj->brightnessObj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(centrolCenterObj->brightnessObj, centrolCenterObj->batteryObj, LV_ALIGN_OUT_TOP_MID, 0, 0);

    centrolCenterObj->brightnessSlider = lv_slider_create(centrolCenterObj->brightnessObj);                                       // 创建bar对象
    lv_obj_set_size(centrolCenterObj->brightnessSlider, LV_CONTROL_BRIGHTNESS_SLIDER_WIDTH, LV_CONTROL_BRIGHTNESS_SLIDER_HEIGHT); // 设置尺寸
    lv_bar_set_value(centrolCenterObj->brightnessSlider, 70, LV_ANIM_OFF);                                                        // 设置初始值
    lv_obj_center(centrolCenterObj->brightnessSlider);
    brightnessValue.minBrightness = lv_slider_get_min_value(centrolCenterObj->brightnessSlider);
    brightnessValue.maxBrightness = lv_slider_get_max_value(centrolCenterObj->brightnessSlider);
    lv_obj_add_event_cb(centrolCenterObj->brightnessSlider, brightness_slider_handle, LV_EVENT_VALUE_CHANGED, NULL);


    centrolCenterObj->brightnessLow = lv_label_create(centrolCenterObj->brightnessObj);
    lv_label_set_text(centrolCenterObj->brightnessLow, LV_SYMBOL_DOWN);
 
    lv_obj_set_style_text_color(centrolCenterObj->brightnessLow, lv_color_white(), 0);
    lv_obj_align_to(centrolCenterObj->brightnessLow, centrolCenterObj->brightnessSlider, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    
    centrolCenterObj->brightnessHigh = lv_label_create(centrolCenterObj->brightnessObj);
    lv_label_set_text(centrolCenterObj->brightnessHigh, LV_SYMBOL_UP);

    lv_obj_set_style_text_color(centrolCenterObj->brightnessHigh, lv_color_white(), 0);
    lv_obj_align_to(centrolCenterObj->brightnessHigh, centrolCenterObj->brightnessSlider, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
}

static void control_center_screen(lv_obj_t *screen)
{
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_style_init(&centrolCenterObj->borderStyle);
    lv_style_set_border_width(&centrolCenterObj->borderStyle, 0);

    status_bar_create(screen);
    middle_content_create(screen);
    battery_create(screen);
    brightness_create(screen);
}

void control_center_start(lv_obj_t *controlCenterScreen)
{
    LV_LOG_USER("control_center_start in");
    lv_obj_set_size(controlCenterScreen, LV_CIRCLE_WATCH, LV_CIRCLE_WATCH);
    lv_obj_set_style_bg_color(controlCenterScreen, lv_color_hex(0x030301), LV_PART_MAIN);
    lv_obj_align(controlCenterScreen, LV_ALIGN_CENTER, 0, -LV_CIRCLE_WATCH);
    lv_obj_set_style_radius(controlCenterScreen, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    if (centrolCenterObj == NULL)
    {
        centrolCenterObj = malloc(sizeof(struct CENTROL_CENTER));
    }

    control_center_screen(controlCenterScreen);
}

void control_center_stop(void)
{
    free(centrolCenterObj);
    centrolCenterObj = NULL;
}
