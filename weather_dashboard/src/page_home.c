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

/* ---- Home page: main weather overview ---- */

typedef struct {
    lv_obj_t *anim_area;       /* Weather animation area */
    lv_obj_t *temp_label;      /* Big temperature */
    lv_obj_t *unit_label;      /* C or F */
    lv_obj_t *cond_label;      /* Condition text */
    lv_obj_t *feels_label;     /* Feels like */
    lv_obj_t *detail_row;      /* Humidity / Wind / Pressure row */
    lv_obj_t *hum_val;
    lv_obj_t *wind_val;
    lv_obj_t *press_val;
    lv_obj_t *chart_card;      /* 24h chart */
    lv_obj_t *forecast_cont;   /* 7-day forecast */
    lv_obj_t *daily_labels[WEATHER_FORECAST];
} home_priv_t;

static home_priv_t g_home;

static lv_obj_t *create_info_card(lv_obj_t *parent, const char *icon,
                                  const char *label_text)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 90, 56);
    lv_obj_set_style_bg_color(card, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 6, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *ic = lv_label_create(card);
    lv_label_set_text(ic, icon);
    lv_obj_set_style_text_color(ic, WD_COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(ic, &lv_font_montserrat_14, 0);

    lv_obj_t *vl = lv_label_create(card);
    lv_label_set_text(vl, "--");
    lv_obj_set_style_text_color(vl, WD_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(vl, &lv_font_montserrat_14, 0);

    lv_obj_t *ll = lv_label_create(card);
    lv_label_set_text(ll, label_text);
    lv_obj_set_style_text_color(ll, WD_COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(ll, &lv_font_montserrat_10, 0);

    /* Return the value label via user_data for easy update */
    lv_obj_set_user_data(card, vl);
    return card;
}

void page_home_create(weather_app_t *app, lv_obj_t *parent)
{
    memset(&g_home, 0, sizeof(g_home));
    weather_model_t *m = &app->model;

    /* Scrollable container */
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(parent, 10, 0);
    lv_obj_set_style_pad_all(parent, 10, 0);

    /* ---- Weather animation area ---- */
    g_home.anim_area = lv_obj_create(parent);
    lv_obj_set_size(g_home.anim_area, lv_pct(100), 70);
    lv_obj_set_style_bg_opa(g_home.anim_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_home.anim_area, 0, 0);
    lv_obj_clear_flag(g_home.anim_area, LV_OBJ_FLAG_SCROLLABLE);
    weather_anim_create(g_home.anim_area, m->current.condition);

    /* ---- Temperature display ---- */
    lv_obj_t *temp_row = lv_obj_create(parent);
    lv_obj_remove_style_all(temp_row);
    lv_obj_set_size(temp_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(temp_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(temp_row, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(temp_row, 4, 0);

    g_home.temp_label = lv_label_create(temp_row);
    lv_obj_set_style_text_font(g_home.temp_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(g_home.temp_label, WD_COLOR_TEXT, 0);

    g_home.unit_label = lv_label_create(temp_row);
    lv_obj_set_style_text_font(g_home.unit_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(g_home.unit_label, WD_COLOR_TEXT_DIM, 0);

    /* Condition */
    g_home.cond_label = lv_label_create(parent);
    lv_obj_set_style_text_font(g_home.cond_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(g_home.cond_label, WD_COLOR_ACCENT, 0);

    /* Feels like */
    g_home.feels_label = lv_label_create(parent);
    lv_obj_set_style_text_font(g_home.feels_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(g_home.feels_label, WD_COLOR_TEXT_DIM, 0);

    /* ---- Info cards row ---- */
    g_home.detail_row = lv_obj_create(parent);
    lv_obj_remove_style_all(g_home.detail_row);
    lv_obj_set_size(g_home.detail_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(g_home.detail_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(g_home.detail_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *hum_card  = create_info_card(g_home.detail_row,
                                           LV_SYMBOL_CHARGE, "Humidity");
    lv_obj_t *wind_card = create_info_card(g_home.detail_row,
                                           LV_SYMBOL_SHUFFLE, "Wind");
    lv_obj_t *pres_card = create_info_card(g_home.detail_row,
                                           LV_SYMBOL_DOWNLOAD, "Pressure");
    g_home.hum_val   = (lv_obj_t *)lv_obj_get_user_data(hum_card);
    g_home.wind_val  = (lv_obj_t *)lv_obj_get_user_data(wind_card);
    g_home.press_val = (lv_obj_t *)lv_obj_get_user_data(pres_card);

    /* ---- 24h Chart ---- */
    g_home.chart_card = chart_widget_create(parent, m);

    /* ---- 7-day forecast ---- */
    g_home.forecast_cont = lv_obj_create(parent);
    lv_obj_set_size(g_home.forecast_cont, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(g_home.forecast_cont, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(g_home.forecast_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(g_home.forecast_cont, 16, 0);
    lv_obj_set_style_border_width(g_home.forecast_cont, 0, 0);
    lv_obj_set_style_pad_all(g_home.forecast_cont, 10, 0);
    lv_obj_set_flex_flow(g_home.forecast_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(g_home.forecast_cont, 6, 0);

    lv_obj_t *fc_title = lv_label_create(g_home.forecast_cont);
    lv_label_set_text(fc_title, LV_SYMBOL_LIST " 7-Day Forecast");
    lv_obj_set_style_text_color(fc_title, WD_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(fc_title, &lv_font_montserrat_14, 0);

    int i;
    for (i = 0; i < WEATHER_FORECAST; i++) {
        lv_obj_t *row = lv_obj_create(g_home.forecast_cont);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        g_home.daily_labels[i] = lv_label_create(row);
        lv_obj_set_style_text_font(g_home.daily_labels[i],
                                   &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(g_home.daily_labels[i],
                                    WD_COLOR_TEXT, 0);
    }

    /* Initial update */
    page_home_update(app);
}

void page_home_update(weather_app_t *app)
{
    weather_model_t *m = &app->model;
    char buf[64];

    /* Temperature */
    weather_temp_str(m->current.temp, m->use_fahrenheit, buf, sizeof(buf));
    lv_label_set_text(g_home.temp_label, buf);
    lv_label_set_text(g_home.unit_label, m->use_fahrenheit ? "°F" : "°C");

    /* Condition */
    snprintf(buf, sizeof(buf), "%s  %s",
             weather_cond_icon(m->current.condition),
             weather_cond_str(m->current.condition));
    lv_label_set_text(g_home.cond_label, buf);

    /* Feels like */
    char tbuf[16];
    weather_temp_str(m->current.feels_like, m->use_fahrenheit,
                     tbuf, sizeof(tbuf));
    snprintf(buf, sizeof(buf), "Feels like %s%s",
             tbuf, m->use_fahrenheit ? "°F" : "°C");
    lv_label_set_text(g_home.feels_label, buf);

    /* Info cards */
    snprintf(buf, sizeof(buf), "%d%%", m->current.humidity);
    lv_label_set_text(g_home.hum_val, buf);

    snprintf(buf, sizeof(buf), "%d.%dm/s",
             m->current.wind_speed / 10, m->current.wind_speed % 10);
    lv_label_set_text(g_home.wind_val, buf);

    snprintf(buf, sizeof(buf), "%dhPa", m->current.pressure);
    lv_label_set_text(g_home.press_val, buf);

    /* Chart */
    chart_widget_update(g_home.chart_card, m);

    /* Animation */
    weather_anim_update(g_home.anim_area, m->current.condition);

    /* 7-day forecast */
    int i;
    for (i = 0; i < WEATHER_FORECAST; i++) {
        char hi[16], lo[16];
        weather_temp_str(m->daily[i].temp_high, m->use_fahrenheit,
                         hi, sizeof(hi));
        weather_temp_str(m->daily[i].temp_low, m->use_fahrenheit,
                         lo, sizeof(lo));
        snprintf(buf, sizeof(buf), "%s   %s   %s / %s",
                 weather_dow_str(m->daily[i].day_of_week),
                 weather_cond_icon(m->daily[i].condition),
                 hi, lo);
        lv_label_set_text(g_home.daily_labels[i], buf);
    }
}

void page_home_destroy(weather_app_t *app)
{
    (void)app;
    memset(&g_home, 0, sizeof(g_home));
}
