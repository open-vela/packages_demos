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
    lv_obj_t *arc;
    lv_obj_t *value_label;
    lv_obj_t *title_label;
    int32_t   min_val;
    int32_t   max_val;
} gauge_priv_t;

lv_obj_t *gauge_widget_create(lv_obj_t *parent, const char *title,
                              int32_t min_val, int32_t max_val,
                              lv_color_t color)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 110, 130);
    lv_obj_set_style_bg_color(cont, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(cont, 16, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 8, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *arc = lv_arc_create(cont);
    lv_obj_set_size(arc, 80, 80);
    lv_obj_align(arc, LV_ALIGN_TOP_MID, 0, 0);
    lv_arc_set_range(arc, (int16_t)min_val, (int16_t)max_val);
    lv_arc_set_value(arc, (int16_t)min_val);
    lv_arc_set_bg_angles(arc, 135, 45);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(arc, WD_COLOR_CARD_LIGHT, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);

    lv_obj_t *vl = lv_label_create(cont);
    lv_label_set_text(vl, "--");
    lv_obj_set_style_text_font(vl, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(vl, WD_COLOR_TEXT, 0);
    lv_obj_align_to(vl, arc, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *tl = lv_label_create(cont);
    lv_label_set_text(tl, title);
    lv_obj_set_style_text_font(tl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(tl, WD_COLOR_TEXT_DIM, 0);
    lv_obj_align(tl, LV_ALIGN_BOTTOM_MID, 0, -2);

    gauge_priv_t *priv = lv_malloc(sizeof(gauge_priv_t));
    priv->arc = arc; priv->value_label = vl;
    priv->title_label = tl;
    priv->min_val = min_val; priv->max_val = max_val;
    lv_obj_set_user_data(cont, priv);
    return cont;
}

void gauge_widget_set_value(lv_obj_t *gauge, int32_t value)
{
    gauge_priv_t *priv = lv_obj_get_user_data(gauge);
    if (!priv) return;
    int32_t v = LV_CLAMP(priv->min_val, value, priv->max_val);
    lv_arc_set_value(priv->arc, (int16_t)v);
    char buf[16];
    snprintf(buf, sizeof(buf), "%" PRId32, v);
    lv_label_set_text(priv->value_label, buf);
    lv_obj_align_to(priv->value_label, priv->arc, LV_ALIGN_CENTER, 0, 0);
}
