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

/*********************
 *      INCLUDES
 *********************/

#include "clock.h"
#include <time.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void clock_get_value(clock_value_t* clock_value)
{
    struct tm* t;
    time_t tt;
    time(&tt);
    t = localtime(&tt);

    clock_value->year = t->tm_year + 1900;
    clock_value->month = t->tm_mon + 1;
    clock_value->date = t->tm_mday;
    clock_value->week = t->tm_wday;
    clock_value->hour = t->tm_hour;
    clock_value->min = t->tm_min;
    clock_value->sec = t->tm_sec;
    clock_value->ms = 0;
}

void clock_set_value(const clock_value_t* clock_value)
{
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
