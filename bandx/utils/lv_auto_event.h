/*
 * Copyright (C) 2020 Xiaomi Corporation
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
#ifndef __LV_AUTO_EVENT_H
#define __LV_AUTO_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t** obj_p;
    lv_event_code_t event;
    uint32_t delay;
    void* user_data;
} lv_auto_event_data_t;

typedef struct {
    lv_timer_t* timer;
    lv_auto_event_data_t* auto_event_data;
    uint32_t len;
    uint32_t run_index;
} lv_auto_event_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a new auto event.
 * @param auto_event_data Pointer to the data structure containing auto event parameters.
 * @param len Length of the auto event data array.
 * @return A pointer to the newly created auto event, or NULL if creation failed.
 */
lv_auto_event_t* lv_auto_event_create(lv_auto_event_data_t* auto_event_data, uint32_t len);

/**
 * Delete an existing auto event.
 * @param ae Pointer to the auto event to be deleted.
 */
void lv_auto_event_del(lv_auto_event_t* ae);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
}
#endif

#endif
