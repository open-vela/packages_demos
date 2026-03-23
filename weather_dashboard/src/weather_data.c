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

#include "weather_data.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lvgl/lvgl.h>

static const int16_t g_base_temps[WEATHER_HOURS] = {
    180, 175, 170, 168, 165, 163, 165, 170,
    180, 195, 210, 225, 238, 248, 255, 258,
    255, 248, 238, 225, 215, 205, 195, 188
};

static const char *g_cond_names[] = {
    "Sunny", "Cloudy", "Overcast", "Light Rain",
    "Heavy Rain", "Snow", "Thunderstorm", "Foggy"
};

static const char *g_cond_icons[] = {
    LV_SYMBOL_IMAGE, LV_SYMBOL_IMAGE, LV_SYMBOL_IMAGE,
    LV_SYMBOL_CHARGE, LV_SYMBOL_CHARGE, LV_SYMBOL_IMAGE,
    LV_SYMBOL_WARNING, LV_SYMBOL_EYE_CLOSE
};

static const char *g_dow_names[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static uint32_t g_rand_state = 12345;

static int32_t sim_rand(int32_t lo, int32_t hi)
{
    g_rand_state = g_rand_state * 1103515245 + 12345;
    uint32_t r = (g_rand_state >> 16) & 0x7FFF;
    return lo + (int32_t)(r % (uint32_t)(hi - lo + 1));
}

void weather_data_init(weather_model_t *model)
{
    int i;
    memset(model, 0, sizeof(*model));
    strncpy(model->current.city, "Beijing", WEATHER_CITY_LEN - 1);
    model->current.temp       = 235;
    model->current.feels_like = 245;
    model->current.humidity   = 62;
    model->current.pressure   = 1013;
    model->current.wind_speed = 35;
    model->current.wind_dir   = 180;
    model->current.uv_index   = 6;
    model->current.aqi        = 75;
    model->current.condition  = WEATHER_SUNNY;

    for (i = 0; i < WEATHER_HOURS; i++) {
        model->hourly[i].hour      = (uint8_t)i;
        model->hourly[i].temp      = g_base_temps[i] + sim_rand(-15, 15);
        model->hourly[i].humidity  = (uint16_t)(55 + sim_rand(-10, 15));
        model->hourly[i].condition = (i >= 6 && i <= 18)
                                     ? WEATHER_SUNNY : WEATHER_CLOUDY;
    }

    for (i = 0; i < WEATHER_FORECAST; i++) {
        model->daily[i].day_of_week = (uint8_t)((1 + i) % 7);
        model->daily[i].temp_high   = 250 + sim_rand(-30, 40);
        model->daily[i].temp_low    = 160 + sim_rand(-20, 30);
        model->daily[i].humidity    = (uint16_t)(50 + sim_rand(0, 30));
        model->daily[i].condition   = (weather_cond_t)(sim_rand(0, 3));
    }
    model->last_update_tick = lv_tick_get();
}

void weather_data_update(weather_model_t *model)
{
    int i;
    model->current.temp      += sim_rand(-3, 3);
    model->current.humidity  = (uint16_t)LV_CLAMP(20,
        (int)model->current.humidity + sim_rand(-2, 2), 95);
    model->current.pressure  = (uint16_t)LV_CLAMP(980,
        (int)model->current.pressure + sim_rand(-1, 1), 1040);
    model->current.wind_speed = (uint16_t)LV_CLAMP(0,
        (int)model->current.wind_speed + sim_rand(-5, 5), 200);
    model->current.aqi       = (uint8_t)LV_CLAMP(0,
        (int)model->current.aqi + sim_rand(-3, 3), 200);
    for (i = 0; i < WEATHER_HOURS; i++) {
        model->hourly[i].temp += sim_rand(-5, 5);
    }
    model->last_update_tick = lv_tick_get();
}

void weather_temp_str(int16_t temp_x10, bool fahrenheit, char *buf, size_t len)
{
    int16_t t = temp_x10;
    if (fahrenheit) t = (int16_t)(t * 9 / 5 + 320);
    snprintf(buf, len, "%d.%d", t / 10, ((t < 0 ? -t : t) % 10));
}

const char *weather_cond_str(weather_cond_t c)
{
    return c < WEATHER_COND_MAX ? g_cond_names[c] : "Unknown";
}

const char *weather_cond_icon(weather_cond_t c)
{
    return c < WEATHER_COND_MAX ? g_cond_icons[c] : LV_SYMBOL_DUMMY;
}

const char *weather_dow_str(uint8_t d)
{
    return d <= 6 ? g_dow_names[d] : "???";
}
