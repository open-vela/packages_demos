#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "lvgl.h"
#include "expression_calc.h"
#include <cstring>
#define RES_ROOT CONFIG_LVX_CALCULATOR_DATA_ROOT

#ifdef CONFIG_CALCULATOR_USE_BUILTIN_RES
LV_FONT_DECLARE(size_16_normal);
LV_FONT_DECLARE(size_22_bold);
LV_FONT_DECLARE(size_24_normal);
LV_FONT_DECLARE(size_28_normal);
LV_FONT_DECLARE(size_48_normal);
LV_FONT_DECLARE(size_60_bold);
LV_IMG_DECLARE(background);
#endif

class CalculatorState {
public:
    lv_obj_t *label;
    bool clear_on_next_input;
    bool clear_error;

    CalculatorState()
        : label(nullptr), clear_on_next_input(false), clear_error(false) {}
};

// C++ class for Resource
class Resource {
public:
    // Fonts sub-structure
    class Fonts {
    public:
        lv_font_t* size_16_normal;
        lv_font_t* size_22_bold;
        lv_font_t* size_24_normal;
        lv_font_t* size_28_normal;
        lv_font_t* size_48_normal;
        lv_font_t* size_60_bold;

        Fonts()
            : size_16_normal(nullptr), size_22_bold(nullptr),
              size_24_normal(nullptr), size_28_normal(nullptr),
              size_48_normal(nullptr), size_60_bold(nullptr) {}
    };

    // Images sub-structure
    class Images {
    public:
#ifdef CONFIG_CALCULATOR_USE_BUILTIN_RES
        const void* background;
#else
        std::string background;
#endif

        Images() : background(
#ifdef CONFIG_CALCULATOR_USE_BUILTIN_RES
            NULL
#else
            ""
#endif
        ) {}
    };

    // Member variables
    Fonts fonts;
    Images images;

    // Constructor to initialize resources
    Resource() {}

    // Method to load the resources (fonts and images)
    bool load_resources() {
#ifdef CONFIG_CALCULATOR_USE_BUILTIN_RES
        fonts.size_16_normal = (lv_font_t*)&size_16_normal;
        
        fonts.size_22_bold = (lv_font_t*)&size_22_bold;
        fonts.size_24_normal = (lv_font_t*)&size_24_normal;
        fonts.size_28_normal = (lv_font_t*)&size_28_normal;
        fonts.size_48_normal = (lv_font_t*)&size_48_normal;
        fonts.size_60_bold = (lv_font_t*)&size_60_bold;
        images.background = &background;
        
#else
        std::string icons_path = get_icons_path();
        std::string fonts_path = get_fonts_path();

        // Font initialization using std::string for paths
        fonts.size_16_normal = lv_freetype_font_create((fonts_path + "/MiSans-Normal.ttf").c_str(), LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 16, LV_FREETYPE_FONT_STYLE_NORMAL);
        fonts.size_22_bold = lv_freetype_font_create((fonts_path + "/MiSans-Semibold.ttf").c_str(), LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 22, LV_FREETYPE_FONT_STYLE_NORMAL);
        fonts.size_24_normal = lv_freetype_font_create((fonts_path + "/MiSans-Normal.ttf").c_str(), LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 24, LV_FREETYPE_FONT_STYLE_NORMAL);
        fonts.size_28_normal = lv_freetype_font_create((fonts_path + "/MiSans-Normal.ttf").c_str(), LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 28, LV_FREETYPE_FONT_STYLE_NORMAL);
        fonts.size_48_normal = lv_freetype_font_create((fonts_path + "/MiSans-Normal.ttf").c_str(), LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 48, LV_FREETYPE_FONT_STYLE_NORMAL);
        fonts.size_60_bold = lv_freetype_font_create((fonts_path + "/MiSans-Semibold.ttf").c_str(), LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 60, LV_FREETYPE_FONT_STYLE_NORMAL);

        // Image initialization
        images.background = icons_path + "/background.png";
        printf("icons Path: %s\n", images.background.c_str());
#endif
        // Check if fonts were loaded successfully
        if (!fonts.size_16_normal || !fonts.size_22_bold || !fonts.size_24_normal || !fonts.size_28_normal || !fonts.size_48_normal || !fonts.size_60_bold ) {
            return false;
        }
        
        return true;
    }

private:
    // Function to get the resource path
    std::string get_res_path() {
        return std::string(RES_ROOT) + "/res";
    }

    // Function to get the fonts path
    std::string get_fonts_path() {
        return get_res_path() + "/fonts";
    }

    // Function to get the icons path
    std::string get_icons_path() {
        return get_res_path() + "/icons";
    }
};

// Function to create the calculator UI
lv_draw_buf_t* calculator_create(lv_obj_t *parent, CalculatorState* state, Resource* R);
void cleanup_resources(lv_draw_buf_t *draw_buf);
#endif // CALCULATOR_H
