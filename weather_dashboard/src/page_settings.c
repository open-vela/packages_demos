/*
 * Copyright (C) 2024 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "weather_ui.h"
#include <stdio.h>
#include <string.h>

/* ---- Settings page ---- */

typedef struct {
    lv_obj_t *sw_unit;
    lv_obj_t *city_dd;
    lv_obj_t *refresh_slider;
    lv_obj_t *refresh_label;
    lv_obj_t *about_label;
} settings_priv_t;

static settings_priv_t g_settings;

static const char *g_cities[] = {
    "Beijing", "Shanghai", "Shenzhen", "Guangzhou",
    "Hangzhou", "Chengdu", "Wuhan", "Nanjing"
};
#define NUM_CITIES (sizeof(g_cities) / sizeof(g_cities[0]))

static void unit_switch_cb(lv_event_t *e)
{
    weather_app_t *app = (weather_app_t *)lv_event_get_user_data(e);
    lv_obj_t *sw = lv_event_get_target(e);
    app->model.use_fahrenheit = lv_obj_has_state(sw, LV_STATE_CHECKED);
}

static void city_dd_cb(lv_event_t *e)
{
    weather_app_t *app = (weather_app_t *)lv_event_get_user_data(e);
    lv_obj_t *dd = lv_event_get_target(e);
    uint32_t sel = lv_dropdown_get_selected(dd);
    if (sel < NUM_CITIES) {
        strncpy(app->model.current.city, g_cities[sel],
                WEATHER_CITY_LEN - 1);
        if (app->city_label) {
            lv_label_set_text(app->city_label, app->model.current.city);
        }
    }
}

static void refresh_slider_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    char buf[32];
    snprintf(buf, sizeof(buf), "Refresh: %"PRId32"s", val);
    lv_label_set_text(g_settings.refresh_label, buf);
}

static lv_obj_t *create_setting_row(lv_obj_t *parent, const char *label)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(row, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(row, 12, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 12, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, label);
    lv_obj_set_style_text_color(lbl, WD_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);

    return row;
}

void page_settings_create(weather_app_t *app, lv_obj_t *parent)
{
    memset(&g_settings, 0, sizeof(g_settings));

    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_style_pad_row(parent, 8, 0);

    /* Title */
    lv_obj_t *title = lv_label_create(parent);
    lv_label_set_text(title, LV_SYMBOL_SETTINGS " Settings");
    lv_obj_set_style_text_color(title, WD_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);

    /* ---- Temperature unit ---- */
    lv_obj_t *unit_row = create_setting_row(parent, "Fahrenheit");
    g_settings.sw_unit = lv_switch_create(unit_row);
    lv_obj_set_size(g_settings.sw_unit, 50, 26);
    lv_obj_set_style_bg_color(g_settings.sw_unit, WD_COLOR_CARD_LIGHT, 0);
    lv_obj_set_style_bg_color(g_settings.sw_unit, WD_COLOR_PRIMARY,
                              LV_PART_INDICATOR | LV_STATE_CHECKED);
    if (app->model.use_fahrenheit) {
        lv_obj_add_state(g_settings.sw_unit, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(g_settings.sw_unit, unit_switch_cb,
                        LV_EVENT_VALUE_CHANGED, app);

    /* ---- City selection ---- */
    lv_obj_t *city_row = create_setting_row(parent, "City");
    g_settings.city_dd = lv_dropdown_create(city_row);
    lv_dropdown_set_options(g_settings.city_dd,
        "Beijing\nShanghai\nShenzhen\nGuangzhou\n"
        "Hangzhou\nChengdu\nWuhan\nNanjing");
    lv_obj_set_width(g_settings.city_dd, 130);
    lv_obj_set_style_bg_color(g_settings.city_dd, WD_COLOR_CARD_LIGHT, 0);
    lv_obj_set_style_text_color(g_settings.city_dd, WD_COLOR_TEXT, 0);
    lv_obj_set_style_border_color(g_settings.city_dd, WD_COLOR_DIVIDER, 0);
    lv_obj_add_event_cb(g_settings.city_dd, city_dd_cb,
                        LV_EVENT_VALUE_CHANGED, app);

    /* ---- Refresh interval ---- */
    lv_obj_t *ref_card = lv_obj_create(parent);
    lv_obj_set_size(ref_card, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(ref_card, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(ref_card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ref_card, 12, 0);
    lv_obj_set_style_border_width(ref_card, 0, 0);
    lv_obj_set_style_pad_all(ref_card, 12, 0);
    lv_obj_set_flex_flow(ref_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(ref_card, 8, 0);
    lv_obj_clear_flag(ref_card, LV_OBJ_FLAG_SCROLLABLE);

    g_settings.refresh_label = lv_label_create(ref_card);
    lv_label_set_text(g_settings.refresh_label, "Refresh: 5s");
    lv_obj_set_style_text_color(g_settings.refresh_label, WD_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(g_settings.refresh_label,
                               &lv_font_montserrat_14, 0);

    g_settings.refresh_slider = lv_slider_create(ref_card);
    lv_obj_set_width(g_settings.refresh_slider, lv_pct(100));
    lv_slider_set_range(g_settings.refresh_slider, 1, 30);
    lv_slider_set_value(g_settings.refresh_slider, 5, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_settings.refresh_slider,
                              WD_COLOR_CARD_LIGHT, 0);
    lv_obj_set_style_bg_color(g_settings.refresh_slider,
                              WD_COLOR_PRIMARY, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(g_settings.refresh_slider,
                              WD_COLOR_ACCENT, LV_PART_KNOB);
    lv_obj_add_event_cb(g_settings.refresh_slider, refresh_slider_cb,
                        LV_EVENT_VALUE_CHANGED, app);

    /* ---- About ---- */
    lv_obj_t *about_card = lv_obj_create(parent);
    lv_obj_set_size(about_card, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(about_card, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(about_card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(about_card, 12, 0);
    lv_obj_set_style_border_width(about_card, 0, 0);
    lv_obj_set_style_pad_all(about_card, 12, 0);
    lv_obj_clear_flag(about_card, LV_OBJ_FLAG_SCROLLABLE);

    g_settings.about_label = lv_label_create(about_card);
    lv_label_set_text(g_settings.about_label,
        "Weather Dashboard v1.0\n"
        "OpenVela Demo Application\n"
        "Built with LVGL v9.x\n\n"
        "Data is simulated for\n"
        "demonstration purposes.");
    lv_obj_set_style_text_color(g_settings.about_label, WD_COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(g_settings.about_label,
                               &lv_font_montserrat_12, 0);
    lv_label_set_long_mode(g_settings.about_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(g_settings.about_label, lv_pct(100));
}

void page_settings_update(weather_app_t *app)
{
    (void)app;
    /* Settings page is mostly static, no periodic update needed */
}

void page_settings_destroy(weather_app_t *app)
{
    (void)app;
    memset(&g_settings, 0, sizeof(g_settings));
}
