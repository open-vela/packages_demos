#include "calculator.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h> 
#include <ctype.h>

static lv_obj_t *label;
static char expr[64] = "";  // 输入表达式

// 计算器函数
static double calculate(const char *expression) {
    double result = 0;
    double num = 0;
    char op = '+'; // 默认加
    const char *ptr = expression;
    bool expect_number = true;

    while (*ptr != '\0') {
        while (*ptr == ' ') ptr++;

        int sign = 1;
        if (*ptr == '-') {
            sign = -1;
            ptr++;
        }

        if (isdigit(*ptr) || *ptr == '.') {
            if (sscanf(ptr, "%lf", &num) != 1) {
                return NAN;
            }
            num *= sign;

            while (isdigit(*ptr) || *ptr == '.') ptr++;

            switch (op) {
                case '+': result += num; break;
                case '-': result -= num; break;
                case '*': result *= num; break;
                case '/': if (num == 0) return NAN; result /= num; break;
                case '%': if (result < 0 || num < 0 || floor(result) != result || floor(num) != num) {
                            return NAN;}
                            result = (int)result % (int)num;
                            break;
                default: return NAN;
            }
            expect_number = false;
        } else if (strchr("+-*/%", *ptr)) {
            if (expect_number && *ptr != '-') return NAN;
            op = *ptr;
            ptr++;
            expect_number = true;
        } else {
            return NAN;
        }
    }

    return result;
}


static void btn_event_cb(lv_event_t *e) {
    const char *txt = lv_event_get_user_data(e);
    static bool just_calculated = false;

    if (strcmp(txt, "C") == 0) {
        expr[0] = '\0';
        just_calculated = false;
    } else if (strcmp(txt, "=") == 0) {
        double result = calculate(expr);
        if (isfinite(result)) {
            snprintf(expr, sizeof(expr), "%.15g", result);
            just_calculated = true;
        } else {
            snprintf(expr, sizeof(expr), "ERROR!");
            just_calculated = false;
        }
    } else if (strcmp(txt, "del") == 0) {
        if (strcmp(expr, "ERROR!") == 0) {
            expr[0] = '\0';
        } else {
            size_t len = strlen(expr);
            if (len > 0) {
                expr[len - 1] = '\0';
            }
        }
        just_calculated = false;
    } else if (strcmp(txt, "sqrt") == 0) {
        double val = calculate(expr);
        if (val < 0) {
            snprintf(expr, sizeof(expr), "ERROR!");
            just_calculated = false;
        } else {
            val = sqrt(val);
            snprintf(expr, sizeof(expr), "%.15g", val);
            just_calculated = true;
        }
    } else if (strcmp(txt, ".") == 0) {
        // 检查是否已经包含小数点
        if (strchr(expr, '.') == NULL) {
            // 没有小数点时，才允许添加
            strcat(expr, txt);
        }
    } else {
        // 如果是刚计算完，而且当前输入是数字，就清空
        if (just_calculated && strcmp(expr, "ERROR!") != 0 && isdigit(txt[0])) {
            expr[0] = '\0';
            just_calculated = false;
        }

        // 输入运算符不清空
        if (!isdigit(txt[0])) {
            just_calculated = false;
        }

        if (strcmp(expr, "ERROR!") == 0) {
            expr[0] = '\0';
        }

        if (strlen(expr) < sizeof(expr) - 1) {
            strcat(expr, txt);
        }
    }

    lv_label_set_text(label, expr);
}



// 创建按钮
static void create_button(lv_obj_t *parent, const char *txt, int col, int row) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 180, 100);  
    lv_obj_align(btn, LV_ALIGN_CENTER, col * 200 - 305, row * 120 - 150);  
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, (void *)txt);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, txt);
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_32, LV_STATE_DEFAULT);
    lv_obj_center(btn_label);
}

// 创建计算器界面
void calculator_create(lv_obj_t *parent) {
    label = lv_label_create(parent);
    lv_obj_set_width(label, 780); 
    lv_obj_set_height(label, 100);  
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_48, LV_STATE_DEFAULT); 
    lv_obj_set_style_radius(label, 10, 0); 
    lv_obj_set_style_border_width(label, 2, 0); 
    lv_obj_set_style_border_color(label, lv_color_hex(0x000000), 0); 
    lv_obj_set_style_bg_color(label, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT); 
    lv_label_set_text(label, ""); // 清空显示框

    const char *btn_map[5][4] = {
        {"7", "8", "9", "/"},
        {"4", "5", "6", "*"},
        {"1", "2", "3", "-"},
        {"C", "0", "=", "+"},
        {".", "%", "sqrt", "del"}
    };


    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 4; col++) {
            create_button(parent, btn_map[row][col], col, row);
        }
    }
}
