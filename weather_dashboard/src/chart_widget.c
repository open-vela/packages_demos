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

typedef struct {
    lv_obj_t *chart;
    lv_chart_series_t *ser_temp;
    lv_chart_series_t *ser_hum;
} chart_priv_t;

lv_obj_t *chart_widget_create(lv_obj_t *parent, weather_model_t *model)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, lv_pct(100), 200);
    lv_obj_set_style_bg_color(card, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 12, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_IMAGE " 24h Temperature & Humidity");
    lv_obj_set_style_text_color(title, WD_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);

    lv_obj_t *chart = lv_chart_create(card);
    lv_obj_set_size(chart, lv_pct(100), 140);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart, WEATHER_HOURS);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -50, 400);
    lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, 0, 100);
    lv_chart_set_div_line_count(chart, 4, 6);
    lv_obj_set_style_bg_color(chart, WD_COLOR_CARD_LIGHT, 0);
    lv_obj_set_style_bg_opa(chart, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(chart, 0, 0);
    lv_obj_set_style_radius(chart, 8, 0);
    lv_obj_set_style_line_color(chart, WD_COLOR_DIVIDER, 0);
    lv_obj_set_style_pad_all(chart, 8, 0);
    lv_obj_set_style_line_width(chart, 2, LV_PART_ITEMS);
    lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);

    lv_chart_series_t *st = lv_chart_add_series(chart, WD_COLOR_WARM,
                                                 LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_series_t *sh = lv_chart_add_series(chart, WD_COLOR_COOL,
                                                 LV_CHART_AXIS_SECONDARY_Y);
    int i;
    for (i = 0; i < WEATHER_HOURS; i++) {
        lv_chart_set_next_value(chart, st, model->hourly[i].temp);
        lv_chart_set_next_value(chart, sh, (lv_coord_t)model->hourly[i].humidity);
    }

    chart_priv_t *priv = lv_malloc(sizeof(chart_priv_t));
    priv->chart = chart; priv->ser_temp = st; priv->ser_hum = sh;
    lv_obj_set_user_data(card, priv);
    return card;
}

void chart_widget_update(lv_obj_t *card, weather_model_t *model)
{
    chart_priv_t *priv = lv_obj_get_user_data(card);
    if (!priv) return;
    int i;
    for (i = 0; i < WEATHER_HOURS; i++) {
        lv_chart_set_value_by_id(priv->chart, priv->ser_temp, i,
                                 model->hourly[i].temp);
        lv_chart_set_value_by_id(priv->chart, priv->ser_hum, i,
                                 (lv_coord_t)model->hourly[i].humidity);
    }
    lv_chart_refresh(priv->chart);
}
