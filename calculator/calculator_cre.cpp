/* UI
*│
*├── CalculatorState
*│   ├── Label: Text display for input and result
*│   ├── clear_on_next_input: bool flag to clear on next input
*│   ├── clear_error: bool flag to indicate error state
*│
*├── Resource
*│   ├── Fonts: Loaded fonts of different sizes for various UI elements
*│   ├── Images: Paths to icon images like "background"
*│   ├── get_res_path(): Returns the resource directory path
*│   ├── get_fonts_path(): Returns the fonts directory path
*│   ├── get_icons_path(): Returns the icons directory path
*│   ├── load_resources(): Initializes fonts and images; returns false if any resource fails to load
*│
*├── Functions
*│   ├── cleanup_resources(): ensuring no memory leaks occur during UI rendering cleanup.
*│   ├── app_create_main_page(): Creates the main UI page with background image
*│   ├── btn_input_cb(): Button click handler for input; appends the button text to the display
*│   ├── btn_clear_cb(): Button click handler for clearing the display
*│   ├── btn_del_cb(): Button click handler for deleting the last character
*│   ├── calc_btn_cb(): Button click handler for performing calculations
*│   ├── create_button(): Helper function to create buttons dynamically based on a map of button labels
*│   ├── calculator_create(): Main function to create the entire calculator UI, including buttons, display, and events
*│
*├── Button Layout
*│   ├── Input Buttons: A grid of buttons representing calculator digits and functions like "sqrt", "log", "sin", "cos", etc.
*│   ├── Control Buttons: Additional buttons like "Clear", "Delete", and "Calculate"
*│
*├── Button Event Handlers
*│   ├── btn_input_cb: Handles number and operator input, updating the display text
*│   ├── btn_clear_cb: Clears the display when the "Clear" button is pressed
*│   ├── btn_del_cb: Deletes the last character from the display when the "Delete" button is pressed
*│   ├── calc_btn_cb: Handles calculation logic when the "Calculate" button is pressed, evaluates the expression and displays the result
*│
*└── UI Style and Appearance
*    ├── Button Styles: Rounded corners, colors for different button states (default, pressed)
*    ├── Label Styles: Font, border, background color, and opacity settings for input/output display
*    ├── Background Image: A background image set in the main page
*/

#include "calculator_cre.h"


#include "lvgl.h"


void cleanup_resources(lv_draw_buf_t *draw_buf) {
    if (draw_buf != NULL) {
        lv_draw_buf_destroy(draw_buf);
    }
    LV_LOG_INFO("Resources cleaned up.");
}

lv_draw_buf_t* app_create_main_page(lv_obj_t *parent, Resource* R) {
    // Step 1: Create image decoder descriptor
    lv_image_decoder_dsc_t decoder_dsc;
    lv_image_decoder_args_t args = { 0 }; // Custom args if needed
#ifdef CONFIG_CALCULATOR_USE_BUILTIN_RES
    lv_result_t res = lv_image_decoder_open(&decoder_dsc, R->images.background, &args);
    if(res != LV_RESULT_OK) {
        LV_LOG_ERROR("Image decode failed !");
        return NULL;
    }
#else
    lv_result_t res = lv_image_decoder_open(&decoder_dsc, R->images.background.c_str(), &args);
    if(res != LV_RESULT_OK) {
        LV_LOG_ERROR("Image decode failed: %s", R->images.background.c_str());
        return NULL;
    }
#endif

    // Step 2: Get decoded data (decoder_dsc.decoded)
    const lv_draw_buf_t* const_buf = decoder_dsc.decoded;

    // Create a writable copy of the draw buffer
    lv_draw_buf_t* draw_buf = lv_draw_buf_dup(const_buf);

    if(draw_buf == NULL) {
        LV_LOG_ERROR("Failed to duplicate draw buffer.");
        lv_image_decoder_close(&decoder_dsc);
        return NULL;
    }

    // Step 3: Use the decoded image (e.g., draw it to a canvas)
    lv_obj_t* canvas = lv_canvas_create(parent);
    lv_canvas_set_draw_buf(canvas, draw_buf);  // Set the decoded buffer

    lv_obj_clear_flag(canvas, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
    lv_obj_set_style_opa(canvas, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_move_background(canvas);
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(canvas, 1280, 800);

    // Step 4: Clean up after we're done with the buffer
    lv_image_decoder_close(&decoder_dsc);
    return draw_buf;
}



// Button input callback function
static void btn_input_cb(lv_event_t *e) {
    auto btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
    
    // Retrieve the button text from the user data
    auto txt = static_cast<const char*>(lv_obj_get_user_data(btn));
    auto state = static_cast<CalculatorState*>(lv_event_get_user_data(e));  // Get the calculator state

    auto old = lv_label_get_text(state->label);
    std::string buf;

    // Check if the input is one of the special functions or constants (e.g., sin, cos, log, sqrt, PI, E)
    bool is_function_or_constant = 
        strcmp(txt, "sin") == 0 || strcmp(txt, "cos") == 0 || strcmp(txt, "log") == 0 ||
        strcmp(txt, "sqrt") == 0 || strcmp(txt, "PI") == 0 || strcmp(txt, "E") == 0 ||
        strcmp(txt, "(") == 0 || strcmp(txt, ")") == 0;

    bool is_digit_or_func = std::isdigit(txt[0]) || is_function_or_constant;
    bool should_clear = (state->clear_on_next_input && is_digit_or_func) || state->clear_error;

    if (should_clear) {
        buf = txt;  // Start with the current button text
        state->clear_on_next_input = false;
        state->clear_error = false;
    } else {
        buf = std::string(old) + txt;  // Concatenate the old text with new input
        state->clear_on_next_input = false;
    }

    lv_label_set_text(state->label, buf.c_str());  // Update the label with the new text
}


// Callback function for the clear button
static void btn_clear_cb(lv_event_t *e) {
    auto state = static_cast<CalculatorState*>(lv_event_get_user_data(e));
    lv_label_set_text(state->label, "");  // Clear the label text
    state->clear_on_next_input = false;
    state->clear_error = false;
}

// Callback function for the delete button
static void btn_del_cb(lv_event_t *e) {
    auto state = static_cast<CalculatorState*>(lv_event_get_user_data(e));
    auto txt = lv_label_get_text(state->label);
    std::string str(txt);  // Convert to std::string for easier manipulation
    size_t len = str.length();

    // If input should be cleared or there's an error, reset the label
    if (state->clear_on_next_input || state->clear_error) {
        lv_label_set_text(state->label, "");
        state->clear_on_next_input = false;
        state->clear_error = false;
        return;
    }

    // Define keywords to delete as a whole
    const std::vector<std::string> keywords = { "sqrt", "sin", "cos", "log", "PI" };

    // Check if the current text ends with any of the keywords
    for (const auto& kw : keywords) {
        if (len >= kw.length() && str.compare(len - kw.length(), kw.length(), kw) == 0) {
            str.erase(len - kw.length());  // Erase the entire keyword
            lv_label_set_text(state->label, str.c_str());
            return;
        }
    }

    // Otherwise, delete the last character
    if (len > 0) {
        str.pop_back();
        lv_label_set_text(state->label, str.c_str());
    }
}

// Callback function for the calculate button
static void calc_btn_cb(lv_event_t *e) {
    auto state = static_cast<CalculatorState*>(lv_event_get_user_data(e));  // Get the user data passed into the event callback
    auto expr = lv_label_get_text(state->label);
    char result_buf[256];

    // Process the expression to handle negative numbers correctly
    std::string modified_expr = expr;

    // If the first character is '-', prepend '0' for negative number
    if (modified_expr[0] == '-') {
        modified_expr = "0" + modified_expr;  // Change '-2' to '0-2'
    }

    // For other negative numbers in the expression, check if '(-' is found
    size_t pos = 0;
    while ((pos = modified_expr.find("(-", pos)) != std::string::npos) {
        modified_expr.replace(pos, 2, "(0-");  // Change '(-' to '(0-'
        pos += 3;  // Skip over the replaced text "(0-" to avoid infinite loop
    }

    // Detect "PI" and append "()" if not followed by '('
    pos = 0;
    while ((pos = modified_expr.find("PI", pos)) != std::string::npos) {
        if (pos + 2 >= modified_expr.size() || modified_expr[pos + 2] != '(') {
            modified_expr.insert(pos + 2, "()");
            pos += 4;  // Move past "PI()"
        } else {
            pos += 3;  // Skip over "PI("
        }
    }

    // Detect "E" and append "()" if not followed by '('
    pos = 0;
    while ((pos = modified_expr.find("E", pos)) != std::string::npos) {
        if (pos + 1 >= modified_expr.size() || modified_expr[pos + 1] != '(') {
            modified_expr.insert(pos + 1, "()");
            pos += 3;  // Move past the inserted "E()"
        } else {
            pos += 2;  // Already has '(', skip over "E("
        }
    }


    XCLZ::eXpressionCalc calc;
    calc.setExpression(modified_expr);  // Use the modified expression

    const auto rpn = calc.reversePolishNotation();
    if (calc.getError().type != XCLZ::ErrorType::Well) {
        std::snprintf(result_buf, sizeof(result_buf), "Error: %s: %s", calc.errorToString().c_str(), calc.getError().msg.c_str());
        state->clear_on_next_input = true;
        state->clear_error = true;
    } else {
        const auto val = calc.evalNotation(rpn);
        if (calc.getError().type != XCLZ::ErrorType::Well) {
            std::snprintf(result_buf, sizeof(result_buf), "Error: %s: %s", calc.errorToString().c_str(), calc.getError().msg.c_str());
            state->clear_on_next_input = true;
            state->clear_error = true;
        } else {
            std::snprintf(result_buf, sizeof(result_buf), "%.8g", val);  // Format the result
            state->clear_on_next_input = true;
        }
    }

    lv_label_set_text(state->label, result_buf);  // Update label with the result or error message
}


// Helper function to create a button
static void create_button(lv_obj_t *parent, const char *txt, int col, int row, CalculatorState* state, Resource* R) {
    auto btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 100, 80);  // Set button size
    lv_obj_align(btn, LV_ALIGN_CENTER, col * 110 - 220, row * 90 - 80);  // Position the button on the screen

    // Set style properties for the button
    lv_obj_set_style_radius(btn, 30, LV_STATE_DEFAULT); // Rounded corners
    lv_obj_set_style_border_width(btn, 2, LV_STATE_DEFAULT); // Border width
    lv_obj_set_style_border_color(btn, lv_color_hex(0x000000), LV_STATE_DEFAULT); // Border color (black)

    // Set colors based on the row and column
    if (col == 0) {  // First column
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2196F3), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x5E3370), LV_STATE_PRESSED);
    } else if (row == 0 && col != 0) {  // First row except for the first button
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFE4C4), LV_STATE_DEFAULT);  
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xEE82EE), LV_STATE_PRESSED);  
    } else {  // Other buttons
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFAFA), LV_STATE_DEFAULT);  // Default color (light gray)
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xF5F5F5), LV_STATE_PRESSED);  // Pressed color (light gray)
    }

    // Create label for the button and set its text
    auto btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, txt);  // Set the button label text
    lv_obj_set_style_text_font(btn_label, R->fonts.size_22_bold, LV_STATE_DEFAULT);  // Set the font
    if (col == 0) {
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);  // Set label color
    } else if (row == 0 && col != 0) {  // First row except for the first button
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0x4F4F4F), LV_STATE_DEFAULT);  // Set label color
    } else {  // Other buttons
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0x000000), LV_STATE_DEFAULT);  // Set label color
    }

    lv_obj_center(btn_label);  // Center the text inside the button

    // Store the button text in the user data for future reference
    lv_obj_set_user_data(btn, (void*)txt);  // Associate the button's text with the button object

    // Add event callback for the button click
    lv_obj_add_event_cb(btn, btn_input_cb, LV_EVENT_CLICKED, state);
}

// Function to create the entire calculator UI
lv_draw_buf_t*  calculator_create(lv_obj_t *parent, CalculatorState* state, Resource* R) {
    // Initialize resources (fonts and images) by calling load_resources
    if (!R->load_resources()) {
        // Handle error if resources failed to load
        printf("Failed to load resources\n");
    }// Initialize resources (fonts and images)
    auto draw_buf = app_create_main_page(parent, R);  // Create the main page with the background

    // Input display box
    auto screen = lv_scr_act();  // Get the current screen
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF0F0F0), LV_STATE_DEFAULT);  // Set gray background
    state->label = lv_label_create(parent);
    lv_obj_set_size(state->label, 540, 130);  // Set label size
    lv_obj_align(state->label, LV_ALIGN_TOP_MID, 0, 40);  // Align label at the top center
    lv_obj_set_style_text_font(state->label, R->fonts.size_48_normal, LV_STATE_DEFAULT);  // Set label font
    lv_obj_set_style_radius(state->label, 10, 0);  // Set label border radius
    lv_obj_set_style_border_width(state->label, 2, 0);  // Set border width
    lv_obj_set_style_border_color(state->label, lv_color_hex(0x000000), 0);  // Set border color (black)
    lv_obj_set_style_bg_color(state->label, lv_color_hex(0xE1FFFF), LV_STATE_DEFAULT);  // Set background color (light cyan)

    // Ensure the background is fully opaque
    lv_obj_set_style_bg_opa(state->label, LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_label_set_text(state->label, "");  // Clear label text initially

    // Input button matrix
    const char *btn_map[5][5] = {
        {"PI", "log", "sin", "cos", "sqrt"},
        {"E", "7", "8", "9", "+"},
        {"%", "4", "5", "6", "-"},
        {"^", "1", "2", "3", "*"},
        {".", "(", "0", ")", "/"}
    };

    // Create buttons from the button map
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            create_button(parent, btn_map[row][col], col, row, state, R);  // Create each button
        }
    }

    // Calculate button
    auto calc_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(calc_btn, lv_color_hex(0xFF8C00), LV_STATE_DEFAULT); // Purple background
    lv_obj_set_style_radius(calc_btn, 35, LV_STATE_DEFAULT); // Rounded corners
    lv_obj_set_style_border_width(calc_btn, 2, LV_STATE_DEFAULT); // Border width
    lv_obj_set_style_border_color(calc_btn, lv_color_hex(0x000000), LV_STATE_DEFAULT); // Border color (black)
    lv_obj_set_size(calc_btn, 130, 80);  // Set button size
    lv_obj_align(calc_btn, LV_ALIGN_CENTER, -200, -170);  // Position the button on the screen
    auto calc_lbl = lv_label_create(calc_btn);  // Create button label
    lv_label_set_text(calc_lbl, "calculate");
    lv_obj_set_style_text_font(calc_lbl, R->fonts.size_22_bold, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(calc_lbl, lv_color_hex(0xFFFAFA), LV_STATE_DEFAULT);  // Set label color
    lv_obj_center(calc_lbl);  // Center the label
    lv_obj_add_event_cb(calc_btn, calc_btn_cb, LV_EVENT_CLICKED, state);  // Add event callback

    // Clear button (Red)
    auto clear_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(clear_btn, lv_color_hex(0xFF8C00), LV_STATE_DEFAULT);  // Red color
    lv_obj_set_size(clear_btn, 130, 80);
    lv_obj_set_style_radius(clear_btn, 35, LV_STATE_DEFAULT); // Rounded corners
    lv_obj_set_style_border_width(clear_btn, 2, LV_STATE_DEFAULT); // Border width
    lv_obj_set_style_border_color(clear_btn, lv_color_hex(0x000000), LV_STATE_DEFAULT); // Border color (black)
    lv_obj_align(clear_btn, LV_ALIGN_CENTER, 200, -170);  // Position the button on the screen
    auto clear_lbl = lv_label_create(clear_btn);
    lv_label_set_text(clear_lbl, "Clear");
    lv_obj_set_style_text_font(clear_lbl, R->fonts.size_22_bold , LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(clear_lbl, lv_color_hex(0xFFFAFA), LV_STATE_DEFAULT);  // Set label color
    lv_obj_center(clear_lbl);
    lv_obj_add_event_cb(clear_btn, btn_clear_cb, LV_EVENT_CLICKED, state);

    // Delete button (Green)
    auto del_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(del_btn, lv_color_hex(0xFF8C00), LV_STATE_DEFAULT);  // Green color
    lv_obj_set_size(del_btn, 130, 80);
    lv_obj_set_style_radius(del_btn, 35, LV_STATE_DEFAULT); // Rounded corners
    lv_obj_set_style_border_width(del_btn, 2, LV_STATE_DEFAULT); // Border width
    lv_obj_set_style_border_color(del_btn, lv_color_hex(0x000000), LV_STATE_DEFAULT); // Border color (black)
    lv_obj_align(del_btn, LV_ALIGN_CENTER, 0, -170);  // Position the button on the screen
    auto del_lbl = lv_label_create(del_btn);
    lv_label_set_text(del_lbl, "Del"); // ⌫ 
    lv_obj_set_style_text_font(del_lbl, R->fonts.size_22_bold , LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(del_lbl, lv_color_hex(0xFFFAFA), LV_STATE_DEFAULT);  // Set label color
    lv_obj_center(del_lbl);
    lv_obj_add_event_cb(del_btn, btn_del_cb, LV_EVENT_CLICKED, state);

    return draw_buf;
}
