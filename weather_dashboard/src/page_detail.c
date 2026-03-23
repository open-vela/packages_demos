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

/* ---- Detail page: gauges + extended info ---- */

typedef struct {
    lv_obj_t *gauge_temp;
    lv_obj_t *gauge_hum;
    lv_obj_t *gauge_pres;
    lv_obj_t *gauge_aqi;
    lv_obj_t *uv_bar;
    lv_obj_t *uv_label;
    lv_obj_t *wind_compass;
    lv_obj_t *wind_label;
    lv_obj_t *wind_dir_label;
} detail_priv_t;

static detail_priv_t g_detail;

static lv_obj_t *create_section_title(lv_obj_t *parent, const char *text)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, WD_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_pad_top(lbl, 6, 0);
    return lbl;
}

void page_detail_create(weather_app_t *app, lv_obj_t *parent)
{
    memset(&g_detail, 0, sizeof(g_detail));

    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_style_pad_row(parent, 8, 0);

    /* ---- Gauges row ---- */
    create_section_title(parent, LV_SYMBOL_IMAGE " Environment Gauges");

    lv_obj_t *gauge_row = lv_obj_create(parent);
    lv_obj_remove_style_all(gauge_row);
    lv_obj_set_size(gauge_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(gauge_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(gauge_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(gauge_row, 8, 0);

    g_detail.gauge_temp = gauge_widget_create(gauge_row, "Temp",
                                              -10, 45, WD_COLOR_WARM);
    g_detail.gauge_hum  = gauge_widget_create(gauge_row, "Humidity",
                                              0, 100, WD_COLOR_COOL);
    g_detail.gauge_pres = gauge_widget_create(gauge_row, "Pressure",
                                              980, 1040, WD_COLOR_GREEN);

    /* Second row of gauges */
    lv_obj_t *gauge_row2 = lv_obj_create(parent);
    lv_obj_remove_style_all(gauge_row2);
    lv_obj_set_size(gauge_row2, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(gauge_row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(gauge_row2, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_detail.gauge_aqi = gauge_widget_create(gauge_row2, "AQI",
                                             0, 200, WD_COLOR_YELLOW);

    /* UV Index card */
    lv_obj_t *uv_card = lv_obj_create(gauge_row2);
    lv_obj_set_size(uv_card, 110, 130);
    lv_obj_set_style_bg_color(uv_card, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(uv_card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(uv_card, 16, 0);
    lv_obj_set_style_border_width(uv_card, 0, 0);
    lv_obj_set_style_pad_all(uv_card, 8, 0);
    lv_obj_clear_flag(uv_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(uv_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(uv_card, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *uv_title = lv_label_create(uv_card);
    lv_label_set_text(uv_title, LV_SYMBOL_IMAGE " UV");
    lv_obj_set_style_text_color(uv_title, WD_COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(uv_title, &lv_font_montserrat_12, 0);

    g_detail.uv_label = lv_label_create(uv_card);
    lv_obj_set_style_text_font(g_detail.uv_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(g_detail.uv_label, WD_COLOR_YELLOW, 0);

    g_detail.uv_bar = lv_bar_create(uv_card);
    lv_obj_set_size(g_detail.uv_bar, 80, 8);
    lv_bar_set_range(g_detail.uv_bar, 0, 11);
    lv_obj_set_style_bg_color(g_detail.uv_bar, WD_COLOR_CARD_LIGHT, 0);
    lv_obj_set_style_bg_color(g_detail.uv_bar, WD_COLOR_YELLOW,
                              LV_PART_INDICATOR);
    lv_obj_set_style_radius(g_detail.uv_bar, 4, 0);
    lv_obj_set_style_radius(g_detail.uv_bar, 4, LV_PART_INDICATOR);

    /* Wind card */
    lv_obj_t *wind_card = lv_obj_create(gauge_row2);
    lv_obj_set_size(wind_card, 110, 130);
    lv_obj_set_style_bg_color(wind_card, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(wind_card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(wind_card, 16, 0);
    lv_obj_set_style_border_width(wind_card, 0, 0);
    lv_obj_set_style_pad_all(wind_card, 8, 0);
    lv_obj_clear_flag(wind_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(wind_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(wind_card, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *wind_title = lv_label_create(wind_card);
    lv_label_set_text(wind_title, LV_SYMBOL_SHUFFLE " Wind");
    lv_obj_set_style_text_color(wind_title, WD_COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(wind_title, &lv_font_montserrat_12, 0);

    g_detail.wind_label = lv_label_create(wind_card);
    lv_obj_set_style_text_font(g_detail.wind_label,
                               &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(g_detail.wind_label, WD_COLOR_TEXT, 0);

    g_detail.wind_dir_label = lv_label_create(wind_card);
    lv_obj_set_style_text_font(g_detail.wind_dir_label,
                               &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(g_detail.wind_dir_label,
                                WD_COLOR_TEXT_DIM, 0);

    page_detail_update(app);
}

static const char *wind_dir_name(uint16_t deg)
{
    if (deg < 23)  return "N";
    if (deg < 68)  return "NE";
    if (deg < 113) return "E";
    if (deg < 158) return "SE";
    if (deg < 203) return "S";
    if (deg < 248) return "SW";
    if (deg < 293) return "W";
    if (deg < 338) return "NW";
    return "N";
}

void page_detail_update(weather_app_t *app)
{
    weather_model_t *m = &app->model;
    char buf[32];

    /* Gauges */
    gauge_widget_set_value(g_detail.gauge_temp, m->current.temp / 10);
    gauge_widget_set_value(g_detail.gauge_hum, m->current.humidity);
    gauge_widget_set_value(g_detail.gauge_pres, m->current.pressure);
    gauge_widget_set_value(g_detail.gauge_aqi, m->current.aqi);

    /* UV */
    snprintf(buf, sizeof(buf), "%d", m->current.uv_index);
    lv_label_set_text(g_detail.uv_label, buf);
    lv_bar_set_value(g_detail.uv_bar, m->current.uv_index, LV_ANIM_ON);

    /* Wind */
    snprintf(buf, sizeof(buf), "%d.%d",
             m->current.wind_speed / 10, m->current.wind_speed % 10);
    lv_label_set_text(g_detail.wind_label, buf);

    snprintf(buf, sizeof(buf), "%s (%d" LV_SYMBOL_DUMMY ")",
             wind_dir_name(m->current.wind_dir), m->current.wind_dir);
    lv_label_set_text(g_detail.wind_dir_label, buf);
}

void page_detail_destroy(weather_app_t *app)
{
    (void)app;
    memset(&g_detail, 0, sizeof(g_detail));
}
