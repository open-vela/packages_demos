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

#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WEATHER_HOURS       24
#define WEATHER_FORECAST    7
#define WEATHER_CITY_LEN    32

/* Weather condition types */

typedef enum {
    WEATHER_SUNNY = 0,
    WEATHER_CLOUDY,
    WEATHER_OVERCAST,
    WEATHER_LIGHT_RAIN,
    WEATHER_HEAVY_RAIN,
    WEATHER_SNOW,
    WEATHER_THUNDERSTORM,
    WEATHER_FOGGY,
    WEATHER_COND_MAX
} weather_cond_t;

/* Current weather snapshot */

typedef struct {
    int16_t         temp;           /* Current temperature (x10, e.g. 235 = 23.5C) */
    int16_t         feels_like;     /* Feels-like temperature (x10) */
    uint16_t        humidity;       /* Relative humidity 0-100% */
    uint16_t        pressure;       /* Atmospheric pressure in hPa */
    uint16_t        wind_speed;     /* Wind speed (x10 m/s) */
    uint16_t        wind_dir;       /* Wind direction 0-359 degrees */
    uint8_t         uv_index;       /* UV index 0-11+ */
    uint8_t         aqi;            /* Air quality index 0-500 */
    weather_cond_t  condition;      /* Weather condition */
    char            city[WEATHER_CITY_LEN];
} weather_current_t;

/* Hourly forecast entry */

typedef struct {
    uint8_t         hour;           /* Hour 0-23 */
    int16_t         temp;           /* Temperature (x10) */
    uint16_t        humidity;       /* Humidity 0-100% */
    weather_cond_t  condition;      /* Condition */
} weather_hourly_t;

/* Daily forecast entry */

typedef struct {
    uint8_t         day_of_week;    /* 0=Sun, 1=Mon, ... 6=Sat */
    int16_t         temp_high;      /* High temperature (x10) */
    int16_t         temp_low;       /* Low temperature (x10) */
    uint16_t        humidity;       /* Humidity */
    weather_cond_t  condition;      /* Condition */
} weather_daily_t;

/* Complete weather data model */

typedef struct {
    weather_current_t   current;
    weather_hourly_t    hourly[WEATHER_HOURS];
    weather_daily_t     daily[WEATHER_FORECAST];
    uint32_t            last_update_tick;
    bool                use_fahrenheit;
} weather_model_t;

/**
 * Initialize weather model with simulated data.
 */
void weather_data_init(weather_model_t *model);

/**
 * Simulate sensor update (call periodically).
 */
void weather_data_update(weather_model_t *model);

/**
 * Convert temperature based on unit setting.
 * Returns string like "23.5" into buf.
 */
void weather_temp_str(int16_t temp_x10, bool fahrenheit, char *buf, size_t len);

/**
 * Get weather condition string.
 */
const char *weather_cond_str(weather_cond_t cond);

/**
 * Get weather condition icon (LVGL symbol).
 */
const char *weather_cond_icon(weather_cond_t cond);

/**
 * Get day-of-week short name.
 */
const char *weather_dow_str(uint8_t dow);

#ifdef __cplusplus
}
#endif

#endif /* WEATHER_DATA_H */
