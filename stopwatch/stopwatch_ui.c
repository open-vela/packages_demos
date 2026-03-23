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

#include "stopwatch_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STOPWATCH_MAX_LAPS     10
#define STOPWATCH_UPDATE_MS    33   /* ~30 fps */

/* Color definitions */

#define COLOR_BG        lv_color_hex(0x1A1A2E)
#define COLOR_START     lv_color_hex(0x16C47F)
#define COLOR_STOP      lv_color_hex(0xE74C3C)
#define COLOR_LAP       lv_color_hex(0x3498DB)
#define COLOR_RESET     lv_color_hex(0x95A5A6)
#define COLOR_TEXT      lv_color_hex(0xECF0F1)
#define COLOR_TEXT_DIM  lv_color_hex(0x7F8C8D)

/* ---------- helpers ---------- */

static void format_time(uint32_t ms, char *buf, size_t len)
{
    uint32_t minutes = ms / 60000;
    uint32_t seconds = (ms % 60000) / 1000;
    uint32_t centis  = (ms % 1000) / 10;
    snprintf(buf, len, "%02" PRIu32 ":%02" PRIu32 ".%02" PRIu32,
             minutes, seconds, centis);
}

static void update_display(stopwatch_ui_t *ui)
{
    char buf[16];
    uint32_t ms = ui->state.elapsed_ms;

    if (ui->state.running) {
        ms += lv_tick_get() - ui->state.start_tick;
    }

    uint32_t minutes = ms / 60000;
    uint32_t seconds = (ms % 60000) / 1000;
    uint32_t centis  = (ms % 1000) / 10;

    snprintf(buf, sizeof(buf), "%02" PRIu32 ":%02" PRIu32,
             minutes, seconds);
    lv_label_set_text(ui->time_label, buf);

    snprintf(buf, sizeof(buf), ".%02" PRIu32, centis);
    lv_label_set_text(ui->ms_label, buf);
}

/* ---------- timer callback ---------- */

static void timer_cb(lv_timer_t *timer)
{
    stopwatch_ui_t *ui = (stopwatch_ui_t *)lv_timer_get_user_data(timer);
    if (ui && ui->state.running) {
        update_display(ui);
    }
}

/* ---------- button callbacks ---------- */

static void start_stop_cb(lv_event_t *e)
{
    stopwatch_ui_t *ui = (stopwatch_ui_t *)lv_event_get_user_data(e);
    if (!ui) return;

    if (ui->state.running) {
        /* Stop */
        ui->state.elapsed_ms += lv_tick_get() - ui->state.start_tick;
        ui->state.running = false;
        lv_label_set_text(ui->start_label, LV_SYMBOL_PLAY " Start");
        lv_obj_set_style_bg_color(ui->btn_start, COLOR_START, 0);
        lv_label_set_text(ui->lap_label, LV_SYMBOL_REFRESH " Reset");
        lv_obj_set_style_bg_color(ui->btn_lap, COLOR_RESET, 0);
    } else {
        /* Start */
        ui->state.start_tick = lv_tick_get();
        ui->state.running = true;
        lv_label_set_text(ui->start_label, LV_SYMBOL_STOP " Stop");
        lv_obj_set_style_bg_color(ui->btn_start, COLOR_STOP, 0);
        lv_label_set_text(ui->lap_label, LV_SYMBOL_LOOP " Lap");
        lv_obj_set_style_bg_color(ui->btn_lap, COLOR_LAP, 0);
    }

    update_display(ui);
}

static void lap_reset_cb(lv_event_t *e)
{
    stopwatch_ui_t *ui = (stopwatch_ui_t *)lv_event_get_user_data(e);
    if (!ui) return;

    if (ui->state.running) {
        /* Record lap */
        if (ui->state.lap_count < STOPWATCH_MAX_LAPS) {
            uint32_t current = ui->state.elapsed_ms +
                               (lv_tick_get() - ui->state.start_tick);
            ui->state.laps[ui->state.lap_count] = current;
            ui->state.lap_count++;

            char buf[32];
            char time_str[16];
            format_time(current, time_str, sizeof(time_str));
            snprintf(buf, sizeof(buf), "Lap %d   %s",
                     ui->state.lap_count, time_str);
            lv_obj_t *item = lv_label_create(ui->lap_list);
            lv_label_set_text(item, buf);
            lv_obj_set_style_text_color(item, COLOR_TEXT, 0);
            lv_obj_set_style_text_font(item, &lv_font_montserrat_14, 0);
        }
    } else {
        /* Reset */
        ui->state.elapsed_ms = 0;
        ui->state.running    = false;
        ui->state.lap_count  = 0;
        memset(ui->state.laps, 0, sizeof(ui->state.laps));

        lv_label_set_text(ui->start_label, LV_SYMBOL_PLAY " Start");
        lv_obj_set_style_bg_color(ui->btn_start, COLOR_START, 0);
        lv_label_set_text(ui->lap_label, LV_SYMBOL_LOOP " Lap");
        lv_obj_set_style_bg_color(ui->btn_lap, COLOR_LAP, 0);

        /* Clear lap list */
        lv_obj_clean(ui->lap_list);

        update_display(ui);
    }
}

/* ---------- public API ---------- */

stopwatch_ui_t *stopwatch_ui_create(lv_obj_t *parent)
{
    stopwatch_ui_t *ui = (stopwatch_ui_t *)lv_malloc(sizeof(stopwatch_ui_t));
    if (!ui) return NULL;
    memset(ui, 0, sizeof(stopwatch_ui_t));

    ui->scr = parent;

    /* Background */
    lv_obj_set_style_bg_color(parent, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);

    /* Main time label  MM:SS */
    ui->time_label = lv_label_create(parent);
    lv_label_set_text(ui->time_label, "00:00");
    lv_obj_set_style_text_font(ui->time_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(ui->time_label, COLOR_TEXT, 0);
    lv_obj_align(ui->time_label, LV_ALIGN_TOP_MID, -20, 30);

    /* Centiseconds label  .XX */
    ui->ms_label = lv_label_create(parent);
    lv_label_set_text(ui->ms_label, ".00");
    lv_obj_set_style_text_font(ui->ms_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(ui->ms_label, COLOR_TEXT_DIM, 0);
    lv_obj_align_to(ui->ms_label, ui->time_label, LV_ALIGN_OUT_RIGHT_BOTTOM,
                    4, -4);

    /* ---- Buttons ---- */

    /* Start / Stop button */
    ui->btn_start = lv_btn_create(parent);
    lv_obj_set_size(ui->btn_start, 120, 48);
    lv_obj_align(ui->btn_start, LV_ALIGN_TOP_MID, -70, 110);
    lv_obj_set_style_bg_color(ui->btn_start, COLOR_START, 0);
    lv_obj_set_style_radius(ui->btn_start, 24, 0);
    lv_obj_add_event_cb(ui->btn_start, start_stop_cb, LV_EVENT_CLICKED, ui);

    ui->start_label = lv_label_create(ui->btn_start);
    lv_label_set_text(ui->start_label, LV_SYMBOL_PLAY " Start");
    lv_obj_center(ui->start_label);
    lv_obj_set_style_text_color(ui->start_label, lv_color_white(), 0);

    /* Lap / Reset button */
    ui->btn_lap = lv_btn_create(parent);
    lv_obj_set_size(ui->btn_lap, 120, 48);
    lv_obj_align(ui->btn_lap, LV_ALIGN_TOP_MID, 70, 110);
    lv_obj_set_style_bg_color(ui->btn_lap, COLOR_LAP, 0);
    lv_obj_set_style_radius(ui->btn_lap, 24, 0);
    lv_obj_add_event_cb(ui->btn_lap, lap_reset_cb, LV_EVENT_CLICKED, ui);

    ui->lap_label = lv_label_create(ui->btn_lap);
    lv_label_set_text(ui->lap_label, LV_SYMBOL_LOOP " Lap");
    lv_obj_center(ui->lap_label);
    lv_obj_set_style_text_color(ui->lap_label, lv_color_white(), 0);

    /* ---- Lap list ---- */

    ui->lap_list = lv_obj_create(parent);
    lv_obj_set_size(ui->lap_list, lv_pct(90), lv_pct(45));
    lv_obj_align(ui->lap_list, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_flex_flow(ui->lap_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui->lap_list, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_color(ui->lap_list, lv_color_hex(0x16213E), 0);
    lv_obj_set_style_bg_opa(ui->lap_list, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ui->lap_list, 12, 0);
    lv_obj_set_style_border_width(ui->lap_list, 0, 0);
    lv_obj_set_style_pad_all(ui->lap_list, 10, 0);
    lv_obj_set_style_pad_row(ui->lap_list, 6, 0);
    lv_obj_set_scrollbar_mode(ui->lap_list, LV_SCROLLBAR_MODE_AUTO);

    /* Timer for display updates */
    ui->timer = lv_timer_create(timer_cb, STOPWATCH_UPDATE_MS, ui);

    return ui;
}

void stopwatch_ui_destroy(stopwatch_ui_t *ui)
{
    if (!ui) return;

    if (ui->timer) {
        lv_timer_delete(ui->timer);
        ui->timer = NULL;
    }

    lv_free(ui);
}
