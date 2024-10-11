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
#ifndef __POWER_H
#define __POWER_H

/*********************
 *      INCLUDES
 *********************/

#include <stdbool.h>
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
 * Get the current battery usage percentage.
 * @return The current battery usage as an unsigned integer value.
 */
uint8_t power_get_battery_usage(void);

/**
 * Check if the battery is currently charging.
 * @return Return true if the battery is charging, otherwise return false.
 */
bool power_get_battery_is_charging(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
}
#endif

#endif
