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

#include "weather_ui.h"

#define MAX_PARTICLES 12

static struct {
    lv_obj_t       *particles[MAX_PARTICLES];
    uint8_t         count;
    weather_cond_t  cond;
    lv_obj_t       *parent;
} g_anim;

static void opa_cb(void *var, int32_t val)
{
    lv_obj_set_style_opa((lv_obj_t *)var, (lv_opa_t)val, 0);
}

static void create_rain(lv_obj_t *p, uint8_t n)
{
    uint8_t i;
    for (i = 0; i < n && i < MAX_PARTICLES; i++) {
        lv_obj_t *d = lv_obj_create(p);
        lv_obj_remove_style_all(d);
        lv_obj_set_size(d, 2, 12 + (i % 3) * 4);
        lv_obj_set_style_bg_color(d, WD_COLOR_COOL, 0);
        lv_obj_set_style_bg_opa(d, LV_OPA_70, 0);
        lv_obj_set_style_radius(d, 1, 0);
        lv_obj_set_pos(d, (lv_coord_t)(20 + (i * 23) % 200),
                          (lv_coord_t)(5 + (i * 17) % 40));

        lv_anim_t a; lv_anim_init(&a);
        lv_anim_set_var(&a, d);
        lv_anim_set_values(&a, lv_obj_get_y(d), lv_obj_get_y(d) + 60);
        lv_anim_set_duration(&a, 800 + i * 100);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_start(&a);

        lv_anim_t fa; lv_anim_init(&fa);
        lv_anim_set_var(&fa, d);
        lv_anim_set_values(&fa, LV_OPA_80, LV_OPA_20);
        lv_anim_set_duration(&fa, 800 + i * 100);
        lv_anim_set_repeat_count(&fa, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(&fa, opa_cb);
        lv_anim_start(&fa);

        g_anim.particles[i] = d;
    }
    g_anim.count = n < MAX_PARTICLES ? n : MAX_PARTICLES;
}

static void create_snow(lv_obj_t *p, uint8_t n)
{
    uint8_t i;
    for (i = 0; i < n && i < MAX_PARTICLES; i++) {
        lv_obj_t *d = lv_obj_create(p);
        lv_obj_remove_style_all(d);
        lv_obj_set_size(d, 6, 6);
        lv_obj_set_style_bg_color(d, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(d, LV_OPA_60, 0);
        lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_pos(d, (lv_coord_t)(15 + (i * 29) % 210),
                          (lv_coord_t)(3 + (i * 13) % 30));

        lv_anim_t a; lv_anim_init(&a);
        lv_anim_set_var(&a, d);
        lv_anim_set_values(&a, lv_obj_get_y(d), lv_obj_get_y(d) + 50);
        lv_anim_set_duration(&a, 2000 + i * 200);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_start(&a);

        lv_anim_t sa; lv_anim_init(&sa);
        lv_anim_set_var(&sa, d);
        lv_anim_set_values(&sa, lv_obj_get_x(d) - 8, lv_obj_get_x(d) + 8);
        lv_anim_set_duration(&sa, 1500 + i * 150);
        lv_anim_set_repeat_count(&sa, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_playback_duration(&sa, 1500 + i * 150);
        lv_anim_set_exec_cb(&sa, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_start(&sa);

        g_anim.particles[i] = d;
    }
    g_anim.count = n < MAX_PARTICLES ? n : MAX_PARTICLES;
}

static void create_sun(lv_obj_t *p)
{
    lv_obj_t *g = lv_obj_create(p);
    lv_obj_remove_style_all(g);
    lv_obj_set_size(g, 50, 50);
    lv_obj_align(g, LV_ALIGN_TOP_RIGHT, -20, 5);
    lv_obj_set_style_bg_color(g, WD_COLOR_YELLOW, 0);
    lv_obj_set_style_bg_opa(g, LV_OPA_40, 0);
    lv_obj_set_style_radius(g, LV_RADIUS_CIRCLE, 0);

    lv_anim_t a; lv_anim_init(&a);
    lv_anim_set_var(&a, g);
    lv_anim_set_values(&a, LV_OPA_20, LV_OPA_60);
    lv_anim_set_duration(&a, 2000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_duration(&a, 2000);
    lv_anim_set_exec_cb(&a, opa_cb);
    lv_anim_start(&a);

    g_anim.particles[0] = g;
    g_anim.count = 1;
}

static void clear_all(void)
{
    uint8_t i;
    for (i = 0; i < g_anim.count; i++) {
        if (g_anim.particles[i]) {
            lv_anim_delete(g_anim.particles[i], NULL);
            lv_obj_delete(g_anim.particles[i]);
            g_anim.particles[i] = NULL;
        }
    }
    g_anim.count = 0;
}

void weather_anim_create(lv_obj_t *parent, weather_cond_t cond)
{
    g_anim.parent = parent;
    g_anim.cond = cond;
    g_anim.count = 0;
    switch (cond) {
    case WEATHER_SUNNY:        create_sun(parent);        break;
    case WEATHER_LIGHT_RAIN:   create_rain(parent, 6);    break;
    case WEATHER_HEAVY_RAIN:
    case WEATHER_THUNDERSTORM: create_rain(parent, 12);   break;
    case WEATHER_SNOW:         create_snow(parent, 8);    break;
    default: break;
    }
}

void weather_anim_update(lv_obj_t *parent, weather_cond_t cond)
{
    if (cond != g_anim.cond) {
        clear_all();
        weather_anim_create(parent, cond);
    }
}
