#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "lvgl.h"
#include "expression_calc.h"

struct CalculatorState {
    lv_obj_t *label;
    bool clear_on_next_input;
    bool clear_error;
};
struct ButtonEventData {
    const char *txt;
    CalculatorState* state;
};

void calculator_create(lv_obj_t *parent, CalculatorState* state);

#endif // CALCULATOR_H

