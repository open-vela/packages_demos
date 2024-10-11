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
#ifndef __PARTICLE_SENSOR_H
#define __PARTICLE_SENSOR_H

/*********************
 *      INCLUDES
 *********************/

#include <stdbool.h>

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
 * Get the current heartbeats detected by the particle sensor.
 * @return The current number of beats as a float value.
 */
float particle_sensor_get_beats(void);

/**
 * Get the range of heartbeats detected by the particle sensor.
 * @param min Pointer to a float to store the minimum beat value.
 * @param max Pointer to a float to store the maximum beat value.
 * @return Return true if the operation was successful, otherwise return false.
 */
bool particle_sensor_get_beats_range(float* min, float* max);

/**
 * Reset the beats range for the particle sensor.
 */
void particle_sensor_reset_beats_range(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
}
#endif

#endif
