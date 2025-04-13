/* UI 
*│
*├── CalculatorState
*│   ├── Label: Text display for input and result
*│   ├── ClearOnNextInput: bool flag to clear on next input
*│   ├── ClearError: bool flag to indicate error state
*│
*├── ButtonEventData
*│   ├── Txt: Button text (e.g., "7", "+", etc.)
*│   ├── State: Pointer to CalculatorState to maintain current state
*│
*├── Functions
*│   ├── btn_input_cb: Handles number/operator button clicks, updates label with input
*│   ├── btn_clear_cb: Clears the input/output label and resets flags
*│   ├── btn_del_cb: Deletes last character from the label or resets if in error/clear state
*│   ├── calc_btn_cb: Processes the calculation, evaluates the expression, handles errors
*│   ├── create_button: Creates a button and assigns its corresponding action
*│   ├── calculator_create: Sets up the entire calculator UI with buttons and event handlers
*/

#include "calculator_cre.h"

// Callback function for number and operator buttons
static void btn_input_cb(lv_event_t *e) {
    // Get the ButtonEventData structure from the user data
    ButtonEventData* event_data = (ButtonEventData*)lv_event_get_user_data(e);
    const char *txt = event_data->txt;  // Get the button text
    CalculatorState* state = event_data->state;  // Get the state

    const char *old = lv_label_get_text(state->label);
    static char buf[512];

    // If it's clear input or there's an error, start fresh
    if (state->clear_on_next_input && isdigit(txt[0]) || state->clear_error) {
        snprintf(buf, sizeof(buf), "%s", txt);
        state->clear_on_next_input = false;
    } else {
        snprintf(buf, sizeof(buf), "%s%s", old, txt);
    }

    lv_label_set_text(state->label, buf);
}

// Callback function for the clear button
static void btn_clear_cb(lv_event_t *e) {
    CalculatorState* state = (CalculatorState*)lv_event_get_user_data(e);
    lv_label_set_text(state->label, "");
    state->clear_on_next_input = false;
    state->clear_error = false;
}

// Callback function for the delete button
static void btn_del_cb(lv_event_t *e) {
    CalculatorState* state = (CalculatorState*)lv_event_get_user_data(e);
    const char *txt = lv_label_get_text(state->label);
    size_t len = strlen(txt);

    if (state->clear_on_next_input || state->clear_error) {
        lv_label_set_text(state->label, "");
        state->clear_on_next_input = false;
        state->clear_error = false;
    } else {
        if (len > 0) {
            static char buf[512];
            strncpy(buf, txt, len - 1);
            buf[len - 1] = '\0';
            lv_label_set_text(state->label, buf);
        }
    }
}

// Callback function for the calculate button
static void calc_btn_cb(lv_event_t *e) {
    CalculatorState* state = (CalculatorState*)lv_event_get_user_data(e);  // Get the user data passed into the event callback
    const char *expr = lv_label_get_text(state->label);
    char result_buf[256];

    try {
        XCLZ::eXpressionCalc calc;
        calc.setExpression(expr);

        const auto rpn = calc.reversePolishNotation();
        if (calc.getError().type != XCLZ::ErrorType::Well) {
            throw std::runtime_error(calc.errorToString() + ": " + calc.getError().msg);
        }

        const auto val = calc.evalNotation(rpn);
        if (calc.getError().type != XCLZ::ErrorType::Well) {
            throw std::runtime_error(calc.errorToString() + ": " + calc.getError().msg);
        }

        snprintf(result_buf, sizeof(result_buf), "%.8g", val);
        state->clear_on_next_input  = true;
    } catch (const std::exception &ex) {
        snprintf(result_buf, sizeof(result_buf), "Error: %s", ex.what());
        state->clear_on_next_input  = true;
        state->clear_error = true;
    }


    lv_label_set_text(state->label, result_buf);
}

// Helper function to create a button
static void create_button(lv_obj_t *parent, const char *txt, int col, int row, CalculatorState* state) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 170, 90);  
    lv_obj_align(btn, LV_ALIGN_CENTER, col * 190 - 380, row * 110 - 100);  

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, txt);
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_32, LV_STATE_DEFAULT);
    lv_obj_center(btn_label);

    // Create a structure to hold both the button text and the state
    ButtonEventData *event_data = new ButtonEventData();
    event_data->txt = txt;
    event_data->state = state;

    // Pass the structure as user data to the callback
    lv_obj_add_event_cb(btn, btn_input_cb, LV_EVENT_CLICKED, event_data);
}

// Function to create the entire calculator UI
void calculator_create(lv_obj_t *parent, CalculatorState* state) {
    // Input display box
    state->label = lv_label_create(parent);
    lv_obj_set_size(state->label, 680, 120);
    lv_obj_align(state->label, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_font(state->label, &lv_font_montserrat_48, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(state->label, 10, 0);
    lv_obj_set_style_border_width(state->label, 2, 0);
    lv_obj_set_style_border_color(state->label, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_color(state->label, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_label_set_text(state->label, "");

    // Input button matrix
    const char *btn_map[5][5] = {
        {"7", "8", "9", "/", "sqrt"},
        {"4", "5", "6", "*", "log"},
        {"1", "2", "3", "-", "sin"},
        {"0", ".", "(", ")", "cos"},
        {"PI", "E", "^", "%", "+"}
    };

    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            create_button(parent, btn_map[row][col], col, row, state);
        }
    }

    // Calculate button
    lv_obj_t *calc_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(calc_btn, lv_color_hex(0xA259FF), LV_STATE_DEFAULT); // Purple
    lv_obj_set_size(calc_btn, 150, 80);
    lv_obj_align(calc_btn, LV_ALIGN_TOP_LEFT, 300, 160);
    lv_obj_t *calc_lbl = lv_label_create(calc_btn);

    lv_label_set_text(calc_lbl, "Calculate");
    lv_obj_set_style_text_font(calc_lbl, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_center(calc_lbl);
    lv_obj_add_event_cb(calc_btn, calc_btn_cb, LV_EVENT_CLICKED, state);

    // Clear button
    lv_obj_t *clear_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(clear_btn, lv_color_hex(0xFF0000), LV_STATE_DEFAULT); // Red
    lv_obj_set_size(clear_btn, 150, 80);
    lv_obj_align(clear_btn, LV_ALIGN_TOP_MID, 0, 160);
    lv_obj_t *clear_lbl = lv_label_create(clear_btn);
    lv_label_set_text(clear_lbl, "Clear");
    lv_obj_set_style_text_font(clear_lbl, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_center(clear_lbl);
    lv_obj_add_event_cb(clear_btn, btn_clear_cb, LV_EVENT_CLICKED, state);

    // Delete button
    lv_obj_t *del_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(del_btn, lv_color_hex(0x00C853), LV_STATE_DEFAULT); // Green
    lv_obj_set_size(del_btn, 150, 80);
    lv_obj_align(del_btn, LV_ALIGN_TOP_RIGHT, -300, 160);
    lv_obj_t *del_lbl = lv_label_create(del_btn);
    lv_label_set_text(del_lbl, "Del");
    lv_obj_set_style_text_font(del_lbl, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_center(del_lbl);
    lv_obj_add_event_cb(del_btn, btn_del_cb, LV_EVENT_CLICKED, state);
}
