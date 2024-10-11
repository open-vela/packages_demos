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
#ifndef __BACKLIGHT_H
#define __BACKLIGHT_H

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Get the current value of the backlight.
 * @return The current backlight value.
 */
uint16_t backlight_get_value(void);

/**
 * Set the value of the backlight.
 * @param val The value to set for the backlight.
 */
void backlight_set_value(int16_t val);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
}
#endif

#endif
