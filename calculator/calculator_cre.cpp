/* UI
*│
*├── CalculatorState
*│   ├── Label: Text display for input and result
*│   ├── ClearOnNextInput: bool flag to clear on next input
*│   ├── ClearError: bool flag to indicate error state
*│
*├── Resource
*│   ├── Fonts: Loaded fonts of different sizes for various UI elements
*│   ├── Images: Paths to icon images like "background"
*│   ├── ResourcePath: Path to resources (fonts and icons)
*│
*├── Functions
*│   ├── get_res_path(): Returns the resource directory path
*│   ├── get_fonts_path(): Returns the fonts directory path
*│   ├── get_icons_path(): Returns the icons directory path
*│   ├── init_resource(): Initializes fonts and images; returns false if any resource fails to load
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

// Function to get the resource path
const char* get_res_path() {
    static char path[256];  // Define a sufficiently large character array to store the path
    snprintf(path, sizeof(path), "%s/res", RES_ROOT);
    return path;
}

// Function to get the fonts path
const char* get_fonts_path() {
    static char path[256];
    snprintf(path, sizeof(path), "%s/fonts", get_res_path());
    return path;
}

// Function to get the icons path
const char* get_icons_path() {
    static char path[256];
    snprintf(path, sizeof(path), "%s/icons", get_res_path());
    return path;
}

// Function to initialize resources (fonts and images)
static bool init_resource(resource_s* R)
{
    const char* icons_path = get_icons_path();
    const char* fonts_path = get_fonts_path();
    
    // Font initialization
    R->fonts.size_16.normal = lv_freetype_font_create(fonts_path ? strcat(strdup(fonts_path), "/MiSans-Normal.ttf"): NULL, LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 16, LV_FREETYPE_FONT_STYLE_NORMAL);
    R->fonts.size_22.bold = lv_freetype_font_create(fonts_path ? strcat(strdup(fonts_path), "/MiSans-Semibold.ttf"): NULL, LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 22, LV_FREETYPE_FONT_STYLE_NORMAL);
    R->fonts.size_24.normal = lv_freetype_font_create(fonts_path ? strcat(strdup(fonts_path), "/MiSans-Normal.ttf"): NULL, LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 24, LV_FREETYPE_FONT_STYLE_NORMAL);
    R->fonts.size_28.normal = lv_freetype_font_create(fonts_path ? strcat(strdup(fonts_path), "/MiSans-Semibold.ttf"): NULL, LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 38, LV_FREETYPE_FONT_STYLE_NORMAL);
    R->fonts.size_60.bold = lv_freetype_font_create(fonts_path ? strcat(strdup(fonts_path), "/MiSans-Semibold.ttf"): NULL, LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 60, LV_FREETYPE_FONT_STYLE_NORMAL);
    
    // Image initialization

    R->images.background = icons_path ? strcat(strdup(icons_path), "/background.png"): NULL;

    // Check if fonts were loaded successfully
    if (R->fonts.size_16.normal == NULL ||
        R->fonts.size_22.bold == NULL ||
        R->fonts.size_24.normal == NULL ||
        R->fonts.size_28.normal == NULL ||
        R->fonts.size_60.bold == NULL) {
        return false;  // Return false if fonts failed to load
    }
    
    return true;
}

// Function to create the main page of the application
static void app_create_main_page(lv_obj_t *parent, resource_s* R)
{
    // Create and set the background image
    lv_obj_t* background_image = lv_image_create(parent);
    lv_obj_move_background(background_image);  // Move image to the bottom layer (background)
    lv_image_set_src(background_image, R->images.background);  // Set background image resource
    lv_obj_align(background_image, LV_ALIGN_CENTER, 0, 0);  // Align image to the center of the screen
    lv_obj_set_size(background_image, 1280, 800);  // Set image size
}

// Button input callback function
static void btn_input_cb(lv_event_t *e) {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    
    // Retrieve the button text from the user data
    const char *txt = (const char*)lv_obj_get_user_data(btn);
    CalculatorState* state = (CalculatorState*)lv_event_get_user_data(e);  // Get the calculator state

    const char *old = lv_label_get_text(state->label);
    char buf[512];

    // If it's clear input or there's an error, start fresh
    if (state->clear_on_next_input && isdigit(txt[0]) || state->clear_error) {
        snprintf(buf, sizeof(buf), "%s", txt);  // Start with the current button text
        state->clear_on_next_input = false;
    } else {
        snprintf(buf, sizeof(buf), "%s%s", old, txt);  // Concatenate the old text with new input
    }

    lv_label_set_text(state->label, buf);  // Update the label with the new text
}

// Callback function for the clear button
static void btn_clear_cb(lv_event_t *e) {
    CalculatorState* state = (CalculatorState*)lv_event_get_user_data(e);
    lv_label_set_text(state->label, "");  // Clear the label text
    state->clear_on_next_input = false;
    state->clear_error = false;
}

// Callback function for the delete button
static void btn_del_cb(lv_event_t *e) {
    CalculatorState* state = (CalculatorState*)lv_event_get_user_data(e);
    const char *txt = lv_label_get_text(state->label);
    size_t len = strlen(txt);

    // If it's clear input or there's an error, reset
    if (state->clear_on_next_input || state->clear_error) {
        lv_label_set_text(state->label, "");
        state->clear_on_next_input = false;
        state->clear_error = false;
    } else {
        if (len > 0) {
            static char buf[512];
            strncpy(buf, txt, len - 1);  // Remove the last character
            buf[len - 1] = '\0';
            lv_label_set_text(state->label, buf);  // Update the label with the modified text
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

        snprintf(result_buf, sizeof(result_buf), "%.8g", val);  // Format the result
        state->clear_on_next_input  = true;
    } catch (const std::exception &ex) {
        snprintf(result_buf, sizeof(result_buf), "Error: %s", ex.what());  // Display error message
        state->clear_on_next_input  = true;
        state->clear_error = true;
    }

    lv_label_set_text(state->label, result_buf);  // Update label with the result or error message
}

// Helper function to create a button
static void create_button(lv_obj_t *parent, const char *txt, int col, int row, CalculatorState* state) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 120, 80);  // Set button size
    lv_obj_align(btn, LV_ALIGN_CENTER, col * 140 - 280, row * 100 - 100);  // Position the button on the screen

    // Set style properties for the button
    lv_obj_set_style_radius(btn, 12, LV_STATE_DEFAULT); // Rounded corners
    lv_obj_set_style_border_width(btn, 2, LV_STATE_DEFAULT); // Border width
    lv_obj_set_style_border_color(btn, lv_color_hex(0x000000), LV_STATE_DEFAULT); // Border color (black)
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x696969), LV_STATE_DEFAULT);  // Default color (gray)
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xD3D3D3), LV_STATE_PRESSED);  // Pressed color (light gray)

    // Create label for the button and set its text
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, txt);  // Set the button label text
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_32, LV_STATE_DEFAULT);  // Set the font
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0xECECEC), LV_STATE_DEFAULT);  // Set label color
    lv_obj_center(btn_label);  // Center the text inside the button

    // Store the button text in the user data for future reference
    lv_obj_set_user_data(btn, (void*)txt);  // Associate the button's text with the button object

    // Add event callback for the button click
    lv_obj_add_event_cb(btn, btn_input_cb, LV_EVENT_CLICKED, state);
}

// Function to create the entire calculator UI
void calculator_create(lv_obj_t *parent, CalculatorState* state, resource_s* R) {
    init_resource(R);  // Initialize resources (fonts and images)
    app_create_main_page(parent, R);  // Create the main page with the background

    // Input display box
    lv_obj_t *screen = lv_scr_act();  // Get the current screen
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF0F0F0), LV_STATE_DEFAULT);  // Set gray background
    state->label = lv_label_create(parent);
    lv_obj_set_size(state->label, 680, 100);  // Set label size
    lv_obj_align(state->label, LV_ALIGN_TOP_MID, 0, 40);  // Align label at the top center
    lv_obj_set_style_text_font(state->label, &lv_font_montserrat_48, LV_STATE_DEFAULT);  // Set label font
    lv_obj_set_style_radius(state->label, 10, 0);  // Set label border radius
    lv_obj_set_style_border_width(state->label, 2, 0);  // Set border width
    lv_obj_set_style_border_color(state->label, lv_color_hex(0x000000), 0);  // Set border color (black)
    lv_obj_set_style_bg_color(state->label, lv_color_hex(0xE1FFFF), LV_STATE_DEFAULT);  // Set background color (light cyan)

    // Ensure the background is fully opaque
    lv_obj_set_style_bg_opa(state->label, LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_label_set_text(state->label, "");  // Clear label text initially

    // Input button matrix
    const char *btn_map[5][5] = {
        {"7", "8", "9", "/", "sqrt"},
        {"4", "5", "6", "*", "log"},
        {"1", "2", "3", "-", "sin"},
        {"0", ".", "(", ")", "cos"},
        {"PI", "E", "^", "%", "+"}
    };

    // Create buttons from the button map
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            create_button(parent, btn_map[row][col], col, row, state);  // Create each button
        }
    }

    // Calculate button
    lv_obj_t *calc_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(calc_btn, lv_color_hex(0xA259FF), LV_STATE_DEFAULT); // Purple background
    lv_obj_set_style_radius(calc_btn, 10, LV_STATE_DEFAULT); // Rounded corners
    lv_obj_set_style_border_width(calc_btn, 2, LV_STATE_DEFAULT); // Border width
    lv_obj_set_style_border_color(calc_btn, lv_color_hex(0x000000), LV_STATE_DEFAULT); // Border color (black)
    lv_obj_set_size(calc_btn, 150, 80);  // Set button size
    lv_obj_align(calc_btn, LV_ALIGN_TOP_LEFT, 330, 160);  // Position button
    lv_obj_t *calc_lbl = lv_label_create(calc_btn);  // Create button label
    lv_label_set_text(calc_lbl, "Calculate");
    lv_obj_set_style_text_font(calc_lbl, R->fonts.size_22.bold, LV_STATE_DEFAULT);
    lv_obj_center(calc_lbl);  // Center the label
    lv_obj_add_event_cb(calc_btn, calc_btn_cb, LV_EVENT_CLICKED, state);  // Add event callback

    // Clear button (Red)
    lv_obj_t *clear_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(clear_btn, lv_color_hex(0xFF0000), LV_STATE_DEFAULT);  // Red color
    lv_obj_set_size(clear_btn, 150, 80);
    lv_obj_set_style_radius(clear_btn, 12, LV_STATE_DEFAULT); // Rounded corners
    lv_obj_set_style_border_width(clear_btn, 2, LV_STATE_DEFAULT); // Border width
    lv_obj_set_style_border_color(clear_btn, lv_color_hex(0x000000), LV_STATE_DEFAULT); // Border color (black)
    lv_obj_align(clear_btn, LV_ALIGN_TOP_MID, 0, 160);
    lv_obj_t *clear_lbl = lv_label_create(clear_btn);
    lv_label_set_text(clear_lbl, "Clear");
    lv_obj_set_style_text_font(clear_lbl, R->fonts.size_22.bold , LV_STATE_DEFAULT);
    lv_obj_center(clear_lbl);
    lv_obj_add_event_cb(clear_btn, btn_clear_cb, LV_EVENT_CLICKED, state);

    // Delete button (Green)
    lv_obj_t *del_btn = lv_btn_create(parent);
    lv_obj_set_style_bg_color(del_btn, lv_color_hex(0x00C853), LV_STATE_DEFAULT);  // Green color
    lv_obj_set_size(del_btn, 150, 80);
    lv_obj_set_style_radius(del_btn, 12, LV_STATE_DEFAULT); // Rounded corners
    lv_obj_set_style_border_width(del_btn, 2, LV_STATE_DEFAULT); // Border width
    lv_obj_set_style_border_color(del_btn, lv_color_hex(0x000000), LV_STATE_DEFAULT); // Border color (black)
    lv_obj_align(del_btn, LV_ALIGN_TOP_RIGHT, -330, 160);
    lv_obj_t *del_lbl = lv_label_create(del_btn);
    lv_label_set_text(del_lbl, "Del");
    lv_obj_set_style_text_font(del_lbl, R->fonts.size_22.bold , LV_STATE_DEFAULT);
    lv_obj_center(del_lbl);
    lv_obj_add_event_cb(del_btn, btn_del_cb, LV_EVENT_CLICKED, state);
}
