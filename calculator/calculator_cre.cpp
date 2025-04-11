#include "calculator_cre.h"

static lv_obj_t *label;
static bool clear_on_next_input = false;
static bool clear_error = false;

static void btn_input_cb(lv_event_t *e) {
    const char *txt = (const char *)lv_event_get_user_data(e);
    const char *old = lv_label_get_text(label);
    static char buf[512];

    if (clear_on_next_input && isdigit(txt[0])||clear_error) {
        snprintf(buf, sizeof(buf), "%s", txt);
        clear_on_next_input = false;
    } else {
        snprintf(buf, sizeof(buf), "%s%s", old, txt);
    }

    lv_label_set_text(label, buf);
}

static void btn_clear_cb(lv_event_t *e) {
    lv_label_set_text(label, "");
    clear_on_next_input = false;
    clear_error = false;
}

static void btn_del_cb(lv_event_t *e) {
    const char *txt = lv_label_get_text(label);
    size_t len = strlen(txt);

    if (clear_on_next_input||clear_error) {
        // 如果显示的是结果，按下 Del 按钮时清空屏幕
        lv_label_set_text(label, "");
        clear_on_next_input = false;
        clear_error = false;
    } else {
        // 如果没有显示结果，删除最后一个字符
        if (len > 0) {
            static char buf[512];
            strncpy(buf, txt, len - 1);
            buf[len - 1] = '\0';
            lv_label_set_text(label, buf);
        }
    }
}

static void calc_btn_cb(lv_event_t *e) {
    const char *expr = lv_label_get_text(label);
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
        clear_on_next_input = true;
    } catch (const std::exception &ex) {
        snprintf(result_buf, sizeof(result_buf), "Error: %s", ex.what());
        clear_on_next_input = true;
        clear_error = true;
    }

    lv_label_set_text(label, result_buf);
}

static void create_button(lv_obj_t *parent, const char *txt, int col, int row) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 170, 90);  
    lv_obj_align(btn, LV_ALIGN_CENTER, col * 190 - 380, row * 110 - 100);  

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, txt);
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_32, LV_STATE_DEFAULT);
    lv_obj_center(btn_label);
    lv_obj_add_event_cb(btn, btn_input_cb, LV_EVENT_CLICKED, (void *)txt);
}

void calculator_create(lv_obj_t *parent) {
    // 输入显示框
    label = lv_label_create(parent);
    lv_obj_set_size(label, 680, 120);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_48, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(label, 10, 0);
    lv_obj_set_style_border_width(label, 2, 0);
    lv_obj_set_style_border_color(label, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_color(label, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_label_set_text(label, "");

    // 输入按钮矩阵
    const char *btn_map[5][5] = {
        {"7", "8", "9", "/", "sqrt"},
        {"4", "5", "6", "*", "log"},
        {"1", "2", "3", "-", "sin"},
        {"0", ".", "(", ")", "cos"},
        {"PI", "E", "^", "%", "+"}
    };

    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            create_button(parent, btn_map[row][col], col, row);
        }
    }

    // Calculate 按钮
    lv_obj_t *calc_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(calc_btn, lv_color_hex(0xA259FF), LV_STATE_DEFAULT); // 紫色
    lv_obj_set_size(calc_btn, 150, 80);
    lv_obj_align(calc_btn, LV_ALIGN_TOP_LEFT, 300, 160);
    lv_obj_t *calc_lbl = lv_label_create(calc_btn);

    lv_label_set_text(calc_lbl, "Calculate");
    lv_obj_set_style_text_font(calc_lbl, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_center(calc_lbl);
    lv_obj_add_event_cb(calc_btn, calc_btn_cb, LV_EVENT_CLICKED, NULL);

    // Clear 按钮
    lv_obj_t *clear_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(clear_btn, lv_color_hex(0xFF0000), LV_STATE_DEFAULT); // 设置背景色为红色
    lv_obj_set_size(clear_btn, 150, 80);
    lv_obj_align(clear_btn, LV_ALIGN_TOP_MID, 0, 160);
    lv_obj_t *clear_lbl = lv_label_create(clear_btn);
    lv_label_set_text(clear_lbl, "Clear");
    lv_obj_set_style_text_font(clear_lbl, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_center(clear_lbl);
    lv_obj_add_event_cb(clear_btn, btn_clear_cb, LV_EVENT_CLICKED, NULL);

    // Del 按钮
    lv_obj_t *del_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(del_btn, lv_color_hex(0x00C853), LV_STATE_DEFAULT); // 绿色
    lv_obj_set_size(del_btn, 150, 80);
    lv_obj_align(del_btn, LV_ALIGN_TOP_RIGHT, -300, 160);
    lv_obj_t *del_lbl = lv_label_create(del_btn);
    lv_label_set_text(del_lbl, "Del");
    lv_obj_set_style_text_font(del_lbl, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_center(del_lbl);
    lv_obj_add_event_cb(del_btn, btn_del_cb, LV_EVENT_CLICKED, NULL);
}
