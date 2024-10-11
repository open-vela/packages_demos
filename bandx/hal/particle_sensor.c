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
#include "particle_sensor.h"

#include <stdint.h>
#include <stdlib.h>

/*********************
 *      DEFINES
 *********************/

#define HR_BEATS_MAX 1000

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void particle_sensor_update_range(float beats);

/**********************
 *  STATIC VARIABLES
 **********************/

static int hr_last_beats = 600;
static float hr_beats_min = HR_BEATS_MAX;
static float hr_beats_max = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

float particle_sensor_get_beats(void)
{
    float retval;
    int breats = ((uint16_t)rand() % 600 + 400);
    if (breats > hr_last_beats) {
        hr_last_beats++;
    } else if (breats < hr_last_beats) {
        hr_last_beats--;
    }
    retval = hr_last_beats / 10.0f;

    particle_sensor_update_range(retval);

    return retval;
}

bool particle_sensor_get_beats_range(float* min, float* max)
{
    if (hr_beats_min > HR_BEATS_MAX - 1 || hr_beats_max < 1) {
        return false;
    }

    *min = hr_beats_min;
    *max = hr_beats_max;
    return true;
}

void particle_sensor_reset_beats_range(void)
{
    hr_beats_min = HR_BEATS_MAX;
    hr_beats_max = 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void particle_sensor_update_range(float beats)
{
    if (beats < 1) {
        return;
    }

    if (beats < hr_beats_min) {
        hr_beats_min = beats;
    }
    if (beats > hr_beats_max) {
        hr_beats_max = beats;
    }
}
