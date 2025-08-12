#include <lvgl/lvgl.h>
#include <math.h>

#define LV_CIRCLE_WATCH 455
#define LV_CIRCLE_PANEL 520

typedef struct TIMER_CENTER {
    lv_obj_t *menu;
    lv_obj_t *main_page;
    lv_obj_t *select_page;
    lv_obj_t *run_page;
 
};
// as
typedef struct TIMER_CONTROL{
    lv_obj_t *ticks[90];
    lv_timer_t *tiemr;
    bool finished;
    bool paused;
    int index;
    int32_t ms;
    int32_t curms;

    lv_obj_t *time_bg;
    lv_obj_t *run_op;
    lv_obj_t *run_of;

    lv_obj_t *run_oc_label;
    lv_obj_t *run_op_time_label;
    lv_obj_t *run_of_time_label;
    lv_obj_t *run_op_btn2_label;

    lv_obj_t *run_op_btn1;
    lv_obj_t *run_op_btn2;
    lv_obj_t *run_of_btn1;
    lv_obj_t *run_of_btn2;
};
extern struct TIMER_CENTER *timerCenterObj;

/**********************
 * GLOBAL PROTOTYPES
 **********************/\

//new code
void timer_start();
void timer_app_start();
void initCb(void);
//new code
