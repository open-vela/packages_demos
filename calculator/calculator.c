#include "calculator.h"


static struct cal_s g_cal;


static const char * btnm_map[] = {  " ", " ", "C", "<",  "\n",
                                    "7", "8", "9", "/",  "\n",
                                    "4", "5", "6", "*",  "\n",
                                    "1", "2", "3", "-",  "\n",
                                    "0", ".", "=", "+",  ""};

                                    
static bool is_operator(char c) 
{
    return c == '+' || c == '-' || c == '*' || c == '/';
}


static double calculate_expression(const char *expr, bool *error) 
{
    *error = true;
    double operands[64] = {0};
    char operators[64] = {0};
    int num_operands = 0;
    int num_ops = 0;
    char num_buf[32] = {0};
    int num_pos = 0;

    const char *p = expr;
    while (*p) {
        if (isdigit(*p) || *p == '.') {
            num_buf[num_pos++] = *p++;
            while (isdigit(*p) || *p == '.') {
                if (num_pos >= sizeof(num_buf)-1) return 0;
                num_buf[num_pos++] = *p++;
            }
            num_buf[num_pos] = '\0';
            
            char *endptr;
            operands[num_operands++] = strtod(num_buf, &endptr);
            if (*endptr != '\0') return 0;
            num_pos = 0;
        }
        else if (is_operator(*p)) {
            if (num_ops >= sizeof(operators)-1) return 0;
            operators[num_ops++] = *p++;
        }
        else {
            return 0;
        }
    }

    if (num_operands == 0 || num_ops != num_operands-1) {
        return 0;
    }

    int i = 0;
    while (i < num_ops) {
        if (operators[i] == '*' || operators[i] == '/') {
            double a = operands[i];
            double b = operands[i+1];
            
            if (operators[i] == '/' && b == 0) {
                return 0;
            }
            
            operands[i] = (operators[i] == '*') ? a * b : a / b;
            
            for (int j = i+1; j < num_operands-1; j++) {
                operands[j] = operands[j+1];
            }
            num_operands--;
            
            for (int j = i; j < num_ops-1; j++) {
                operators[j] = operators[j+1];
            }
            num_ops--;
        } else {
            i++;
        }
    }

    double result = operands[0];
    for (i = 0; i < num_ops; i++) {
        if (operators[i] == '+') {
            result += operands[i+1];
        } else {
            result -= operands[i+1];
        }
    }

    *error = false;
    return result;
}


static void cal_btnmatrix_event_cb(lv_event_t *e) 
{
    lv_obj_t * obj = lv_event_get_target(e);
    uint32_t id = lv_buttonmatrix_get_selected_button(obj);
    const char * text = lv_buttonmatrix_get_button_text(obj, id);

    if (0 == strcmp(text,"=")) {
        
        bool error = false;
        double result = calculate_expression(g_cal.calc_exp, &error);
        
        if (!error && g_cal.count > 0) {
            char buf[32];
            if (result == (int)result) {
                snprintf(buf, sizeof(buf), "%d", (int)result);
            } else {
                snprintf(buf, sizeof(buf), "%.6g", result);
            }
            lv_textarea_set_text(g_cal.ui.input, buf);
            g_cal.result = result;
            
            lv_memset(g_cal.calc_exp, 0, sizeof(g_cal.calc_exp));
            snprintf(g_cal.calc_exp, sizeof(g_cal.calc_exp), "%.6g", result);
            g_cal.count = strlen(g_cal.calc_exp);
        } else {
            lv_textarea_set_text(g_cal.ui.input, "Error");
            lv_memset(g_cal.calc_exp, 0, sizeof(g_cal.calc_exp));
            g_cal.count = 0;
        }
        g_cal.cal_status = CAL_COMPLETED;
    }
    else if(0 == strcmp(text,"C")) {
        lv_textarea_set_text(g_cal.ui.input,"");
        lv_memset(g_cal.calc_exp, 0, sizeof(g_cal.calc_exp));
        g_cal.count = 0;
    }
    else if(0 == strcmp(text,"<")) {
        lv_textarea_delete_char(g_cal.ui.input);
        if (g_cal.count > 0) {
            g_cal.calc_exp[--g_cal.count] = '\0';
        }
    }
    else {
        const char *input_text = lv_textarea_get_text(g_cal.ui.input);
        size_t len = strlen(input_text);
        
        bool valid_input = true;        
        
        if( CAL_COMPLETED == g_cal.cal_status )
        {
            lv_textarea_set_text(g_cal.ui.input,"");
            lv_memset(g_cal.calc_exp, 0, sizeof(g_cal.calc_exp));    
            g_cal.count = 0;     
            g_cal.cal_status = CAL_NO_COMPLETED;               
        }
        
        if (len > 0 && is_operator(*text)) {
            char last_char = input_text[len-1];
            if (is_operator(last_char)) {
                valid_input = false;
                return;
            }
        }
        if (len == 0 && is_operator(*text)) {
            valid_input = false;
            return;
        }
        if (*text == '.') {
            char *last_dot = strrchr(g_cal.calc_exp, '.');
            char *last_op = strpbrk(g_cal.calc_exp, "+-*/");
            if (last_dot && (!last_op || last_dot > last_op)) {
                valid_input = false;
                return;
            }
        }
        if (valid_input) {
            lv_textarea_add_text(g_cal.ui.input, text);
            strcat(g_cal.calc_exp, text);
            g_cal.count++;
        }
    }    
}


void cal_app_create(void)
{
    lv_obj_t *root = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 175, 275);
    lv_obj_center(root);

    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    /*Input textarea*/
    lv_obj_t * ta_input = lv_textarea_create(root);
    g_cal.ui.input = ta_input;
    lv_obj_set_style_bg_color(ta_input, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_color(ta_input, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(ta_input, 0, 0);
    lv_obj_set_style_border_width(ta_input, 0, 0);

    lv_obj_set_size(ta_input, LV_PCT(100), LV_PCT(5));
    lv_textarea_set_one_line(ta_input, true);
    lv_textarea_set_cursor_click_pos(ta_input, false);
    lv_textarea_set_max_length(ta_input, 128);
    lv_textarea_set_align(ta_input, LV_TEXT_ALIGN_RIGHT);
    lv_textarea_set_text(ta_input, "");

    /*Calculator input panel*/
    lv_obj_t * btnm = lv_btnmatrix_create(root);
    g_cal.ui.btnm = btnm;
    lv_obj_set_style_radius(btnm, 0, 0);
    lv_obj_set_style_border_width(btnm, 0, 0);
    lv_obj_set_size(btnm, LV_PCT(100), LV_PCT(70));
    lv_buttonmatrix_set_map(btnm, btnm_map);
    lv_obj_add_event_cb(btnm,cal_btnmatrix_event_cb,LV_EVENT_VALUE_CHANGED,root);
}

