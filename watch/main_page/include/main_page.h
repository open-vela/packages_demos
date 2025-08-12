#include "utils.h"

#define CONTER_PAGE         "cntPage"
#define MESSAGE_PAGE        "msgPage"
#define APPS_PAGE           "appsPage"
#define HOME_PAGE           "homePage"
#define CLOCK_PAGE          "clockPage"
#define DIGITAL_PAGE        "digitalPage"
#define TIMER_PAGE          "timerPage"
#define CLOCK_LIST_PAGE     "clockListPage"
#define CLOCK_SELECT_PAGE   "clockSelectPage"
#define INSOMEONEAPP_PAGE    "in_someone_app_home_page"

typedef struct
{
    lv_obj_t *watchScr;
    lv_obj_t *cntCenter;
    lv_obj_t *msgCenter;
    lv_obj_t *appListCenter;
    // lv_obj_t *clockListCenter;
    // lv_obj_t *clockSelectCenter;
    lv_obj_t *mainPage;
    lv_obj_t *hourObj;
    lv_obj_t *minuteObj;
    lv_obj_t *secondObj;
    lv_timer_t *timer;
    char current_page[1024];
    char tagBuf[15];
    //int icon_type;
    // lv_mutex_t lock;
} MainPageData;

//new code
 extern MainPageData *mpd ;
 //new code

void main_page(const char *str);
void sys_gesture_event_cb(lv_event_t *e);