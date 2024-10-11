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
#ifndef __LV_OBJ_EXT_FUNC_H
#define __LV_OBJ_EXT_FUNC_H

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

#define LV_ANIM_TIME_DEFAULT 300
#define LV_ANIM_EXEC(attr) (lv_anim_exec_xcb_t) lv_obj_set_##attr

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Add animation to an object.
 * @param obj pointer to the object.
 * @param a pointer to the animation (can be NULL).
 * @param exec_cb the callback to execute for the animation.
 * @param start the starting value of the animation.
 * @param end the ending value of the animation.
 * @param time the duration of the animation in milliseconds.
 * @param delay the delay before the animation starts in milliseconds.
 * @param ready_cb the callback to call when the animation is ready (can be NULL).
 * @param path_cb the callback for the animation path (can be NULL).
 */
void lv_obj_add_anim(
    lv_obj_t* obj, lv_anim_t* a,
    lv_anim_exec_xcb_t exec_cb,
    int32_t start,
    int32_t end,
    uint16_t time,
    uint32_t delay,
    lv_anim_ready_cb_t ready_cb,
    lv_anim_path_cb_t path_cb);

/**
 * Macro to add animation to an object with specified attributes.
 * @param obj pointer to the object.
 * @param attr the attribute to animate.
 * @param target the target value for the animation.
 * @param time the duration of the animation in milliseconds.
 */
#define LV_OBJ_ADD_ANIM(obj, attr, target, time)   \
    do {                                           \
        lv_obj_add_anim(                           \
            (obj), NULL,                           \
            (lv_anim_exec_xcb_t)lv_obj_set_##attr, \
            lv_obj_get_##attr(obj),                \
            (target),                              \
            (time),                                \
            0,                                     \
            NULL,                                  \
            lv_anim_path_ease_out);                \
    } while (0)

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
}
#endif

#endif
