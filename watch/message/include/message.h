#include <lvgl/lvgl.h>



#define LV_CIRCLE_WATCH 455

struct MESSAGE_CENTER {
    lv_obj_t *messageCenter;
    lv_obj_t *messageCenterLabel;
    lv_obj_t *messageCenterImage;
    lv_obj_t *messageCenterImage2;
};

extern struct MESSAGE_CENTER *messageCenterObj;

void message_center_start(lv_obj_t* messageCenterScreen);