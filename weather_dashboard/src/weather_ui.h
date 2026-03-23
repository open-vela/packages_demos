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

#ifndef WEATHER_UI_H
#define WEATHER_UI_H

#include <lvgl/lvgl.h>
#include "weather_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Color palette ---- */

#define WD_COLOR_BG         lv_color_hex(0x0F0F23)
#define WD_COLOR_CARD       lv_color_hex(0x1A1A3E)
#define WD_COLOR_CARD_LIGHT lv_color_hex(0x252550)
#define WD_COLOR_PRIMARY    lv_color_hex(0x6C63FF)
#define WD_COLOR_ACCENT     lv_color_hex(0x00D2FF)
#define WD_COLOR_WARM       lv_color_hex(0xFF6B6B)
#define WD_COLOR_COOL       lv_color_hex(0x48DBFB)
#define WD_COLOR_GREEN      lv_color_hex(0x2ED573)
#define WD_COLOR_YELLOW     lv_color_hex(0xFFA502)
#define WD_COLOR_TEXT       lv_color_hex(0xECF0F1)
#define WD_COLOR_TEXT_DIM   lv_color_hex(0x7F8C8D)
#define WD_COLOR_DIVIDER    lv_color_hex(0x2C2C54)

/* ---- Page IDs ---- */

typedef enum {
    WD_PAGE_HOME = 0,
    WD_PAGE_DETAIL,
    WD_PAGE_SETTINGS,
    WD_PAGE_MAX
} wd_page_id_t;

/* ---- Forward declarations ---- */

typedef struct weather_app weather_app_t;

/* ---- Page interface ---- */

typedef struct {
    void (*create)(weather_app_t *app, lv_obj_t *parent);
    void (*update)(weather_app_t *app);
    void (*destroy)(weather_app_t *app);
} wd_page_ops_t;

/* ---- Main application context ---- */

struct weather_app {
    lv_obj_t        *scr;               /* Root screen */
    lv_obj_t        *content;           /* Content area (swappable) */
    lv_obj_t        *navbar;            /* Bottom navigation bar */
    lv_obj_t        *nav_btns[WD_PAGE_MAX]; /* Nav buttons */
    lv_obj_t        *nav_labels[WD_PAGE_MAX];
    lv_obj_t        *status_bar;        /* Top status bar */
    lv_obj_t        *city_label;        /* City name in status bar */
    lv_obj_t        *time_label;        /* Time in status bar */

    wd_page_id_t     current_page;      /* Active page */
    lv_obj_t        *page_objs[WD_PAGE_MAX]; /* Page root objects */

    weather_model_t  model;             /* Weather data */
    lv_timer_t      *data_timer;        /* Data refresh timer */
    lv_timer_t      *clock_timer;       /* Clock update timer */
    lv_timer_t      *anim_timer;        /* Animation timer */
};

/* ---- Page creation functions ---- */

void page_home_create(weather_app_t *app, lv_obj_t *parent);
void page_home_update(weather_app_t *app);
void page_home_destroy(weather_app_t *app);

void page_detail_create(weather_app_t *app, lv_obj_t *parent);
void page_detail_update(weather_app_t *app);
void page_detail_destroy(weather_app_t *app);

void page_settings_create(weather_app_t *app, lv_obj_t *parent);
void page_settings_update(weather_app_t *app);
void page_settings_destroy(weather_app_t *app);

/* ---- Widget helpers ---- */

lv_obj_t *chart_widget_create(lv_obj_t *parent, weather_model_t *model);
void chart_widget_update(lv_obj_t *chart, weather_model_t *model);

lv_obj_t *gauge_widget_create(lv_obj_t *parent, const char *title,
                              int32_t min_val, int32_t max_val,
                              lv_color_t color);
void gauge_widget_set_value(lv_obj_t *gauge, int32_t value);

/* ---- Weather animation ---- */

void weather_anim_create(lv_obj_t *parent, weather_cond_t cond);
void weather_anim_update(lv_obj_t *parent, weather_cond_t cond);

#ifdef __cplusplus
}
#endif

#endif /* WEATHER_UI_H */
