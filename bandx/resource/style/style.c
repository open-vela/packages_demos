/**
 * @file style.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void style_empty_init(lv_style_t* style)
{
}

void style_root_def_init(lv_style_t* style)
{
    lv_style_set_bg_color(style, lv_color_black());
    lv_style_set_bg_opa(style, LV_OPA_COVER);
    lv_style_set_size(style, LV_PCT(100), LV_PCT(100));
}

void style_btn_trans_init(lv_style_t* style)
{
    static const lv_style_prop_t props[] = {
        LV_STYLE_BG_COLOR,
        LV_STYLE_TRANSFORM_WIDTH,
        LV_STYLE_TRANSFORM_HEIGHT,
        LV_STYLE_PROP_INV
    };
    static lv_style_transition_dsc_t dsc;
    lv_style_transition_dsc_init(&dsc, props, lv_anim_path_overshoot, 200, 0, NULL);
    lv_style_set_transition(style, &dsc);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
