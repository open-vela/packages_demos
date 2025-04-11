#ifndef __RELATIVES_CAL_H__
#define __RELATIVES_CAL_H__


/*********************
 *      INCLUDES
 *********************/

// include lvgl headers
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <ctype.h>

struct relatives_cal_s
{
    struct 
    {
        lv_obj_t *btnm;
        lv_obj_t *input;
    }ui;
    
    uint16_t         count;                     
    char             calc_exp[128]; 
    double           result;
    uint8_t          cal_status;
};

void relatives_cal_app_create(void);


#endif /* __RELATIVES_CAL_H__ */
