#ifndef __CALCULATOR_H__
#define __CALCULATOR_H__


/*********************
 *      INCLUDES
 *********************/

// include lvgl headers
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <ctype.h>

#define CAL_COMPLETED    1
#define CAL_NO_COMPLETED 0

enum cal_error_e
{
    cal_no_error = 0,
    cal_is_error
};

struct cal_s
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
    enum cal_error_e cal_error;
};

void cal_app_create(void);


#endif /* __CALCULATOR_H__ */
