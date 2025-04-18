#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "lvgl.h"
#include "expression_calc.h"
#define RES_ROOT CONFIG_LVX_CALCULATOR_DATA_ROOT


struct CalculatorState {
    lv_obj_t *label;
    bool clear_on_next_input;
    bool clear_error;
};

struct resource_s {
    struct {
        struct {
            lv_font_t* normal;
        } size_16;
        struct {
            lv_font_t* bold;
        } size_22;
        struct {
            lv_font_t* normal;
        } size_24;
        struct {
            lv_font_t* normal;
        } size_28;
        struct {
            lv_font_t* bold;
        } size_60;
    } fonts;
    struct {
        const char* background;
    } images;
};

void calculator_create(lv_obj_t *parent, CalculatorState* state, resource_s* R);

#endif // CALCULATOR_H

