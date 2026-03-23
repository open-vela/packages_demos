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

#ifndef STOPWATCH_UI_H
#define STOPWATCH_UI_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Stopwatch state */

typedef struct {
    uint32_t elapsed_ms;     /* Total elapsed time in milliseconds */
    bool     running;        /* Whether the stopwatch is running */
    uint32_t start_tick;     /* Tick when started */
    uint8_t  lap_count;      /* Number of recorded laps */
    uint32_t laps[10];       /* Lap times (max 10) */
} stopwatch_state_t;

/* UI elements */

typedef struct {
    lv_obj_t            *scr;          /* Screen object */
    lv_obj_t            *time_label;   /* Main time display */
    lv_obj_t            *ms_label;     /* Milliseconds display */
    lv_obj_t            *btn_start;    /* Start/Stop button */
    lv_obj_t            *btn_lap;      /* Lap/Reset button */
    lv_obj_t            *lap_list;     /* Lap time list */
    lv_obj_t            *start_label;  /* Start button label */
    lv_obj_t            *lap_label;    /* Lap button label */
    lv_timer_t          *timer;        /* Update timer */
    stopwatch_state_t    state;        /* Stopwatch state */
} stopwatch_ui_t;

/**
 * Create the stopwatch UI on the given screen.
 *
 * @param parent  The parent object (usually lv_screen_active())
 * @return Pointer to the allocated stopwatch_ui_t, or NULL on failure
 */

stopwatch_ui_t *stopwatch_ui_create(lv_obj_t *parent);

/**
 * Destroy the stopwatch UI and free resources.
 *
 * @param ui  Pointer to the stopwatch UI
 */

void stopwatch_ui_destroy(stopwatch_ui_t *ui);

#ifdef __cplusplus
}
#endif

#endif /* STOPWATCH_UI_H */
