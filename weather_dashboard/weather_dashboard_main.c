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

#include <nuttx/config.h>
#include <unistd.h>
#include <uv.h>
#include <lvgl/lvgl.h>
#include <string.h>
#include <stdio.h>
#include "src/weather_ui.h"

/* ---- Page operations table ---- */

static const wd_page_ops_t g_page_ops[WD_PAGE_MAX] = {
    [WD_PAGE_HOME]     = { page_home_create,     page_home_update,
                           page_home_destroy },
    [WD_PAGE_DETAIL]   = { page_detail_create,   page_detail_update,
                           page_detail_destroy },
    [WD_PAGE_SETTINGS] = { page_settings_create, page_settings_update,
                           page_settings_destroy },
};

static const char *g_nav_icons[WD_PAGE_MAX] = {
    LV_SYMBOL_HOME, LV_SYMBOL_LIST, LV_SYMBOL_SETTINGS
};

static const char *g_nav_labels[WD_PAGE_MAX] = {
    "Home", "Detail", "Settings"
};

static weather_app_t g_app;

/* ---- Navigation ---- */

static void switch_page(weather_app_t *app, wd_page_id_t page)
{
    if (page >= WD_PAGE_MAX || page == app->current_page) return;

    /* Destroy current page */
    if (g_page_ops[app->current_page].destroy) {
        g_page_ops[app->current_page].destroy(app);
    }
    if (app->page_objs[app->current_page]) {
        lv_obj_clean(app->page_objs[app->current_page]);
        lv_obj_delete(app->page_objs[app->current_page]);
        app->page_objs[app->current_page] = NULL;
    }

    /* Update nav button styles */
    int i;
    for (i = 0; i < WD_PAGE_MAX; i++) {
        lv_color_t c = (i == (int)page) ? WD_COLOR_PRIMARY : WD_COLOR_TEXT_DIM;
        lv_obj_set_style_text_color(app->nav_labels[i], c, 0);
    }

    /* Create new page */
    app->current_page = page;
    lv_obj_t *page_obj = lv_obj_create(app->content);
    lv_obj_set_size(page_obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(page_obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page_obj, 0, 0);
    lv_obj_set_style_pad_all(page_obj, 0, 0);
    app->page_objs[page] = page_obj;

    if (g_page_ops[page].create) {
        g_page_ops[page].create(app, page_obj);
    }

    /* Slide-in animation */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, page_obj);
    lv_anim_set_values(&a, 30, 0);
    lv_anim_set_duration(&a, 200);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);

    /* Fade-in */
    lv_anim_t fa;
    lv_anim_init(&fa);
    lv_anim_set_var(&fa, page_obj);
    lv_anim_set_values(&fa, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&fa, 200);
    lv_anim_set_exec_cb(&fa, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_start(&fa);
}

static void nav_btn_cb(lv_event_t *e)
{
    weather_app_t *app = (weather_app_t *)lv_event_get_user_data(e);
    lv_obj_t *btn = lv_event_get_target(e);

    int i;
    for (i = 0; i < WD_PAGE_MAX; i++) {
        if (app->nav_btns[i] == btn) {
            switch_page(app, (wd_page_id_t)i);
            break;
        }
    }
}

/* ---- Timers ---- */

static void data_timer_cb(lv_timer_t *timer)
{
    weather_app_t *app = (weather_app_t *)lv_timer_get_user_data(timer);
    weather_data_update(&app->model);

    if (g_page_ops[app->current_page].update) {
        g_page_ops[app->current_page].update(app);
    }
}

static void clock_timer_cb(lv_timer_t *timer)
{
    weather_app_t *app = (weather_app_t *)lv_timer_get_user_data(timer);
    uint32_t tick = lv_tick_get() / 1000;
    uint32_t h = (tick / 3600) % 24;
    uint32_t m = (tick / 60) % 60;
    char buf[8];
    snprintf(buf, sizeof(buf), "%02" PRIu32 ":%02" PRIu32, h, m);
    lv_label_set_text(app->time_label, buf);
}

/* ---- App init ---- */

static void app_create(weather_app_t *app)
{
    memset(app, 0, sizeof(*app));
    weather_data_init(&app->model);

    app->scr = lv_screen_active();
    lv_obj_set_style_bg_color(app->scr, WD_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(app->scr, LV_OPA_COVER, 0);
    lv_obj_set_flex_flow(app->scr, LV_FLEX_FLOW_COLUMN);

    /* ---- Status bar ---- */
    app->status_bar = lv_obj_create(app->scr);
    lv_obj_set_size(app->status_bar, lv_pct(100), 36);
    lv_obj_set_style_bg_color(app->status_bar, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(app->status_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(app->status_bar, 0, 0);
    lv_obj_set_style_border_width(app->status_bar, 0, 0);
    lv_obj_set_style_pad_hor(app->status_bar, 12, 0);
    lv_obj_clear_flag(app->status_bar, LV_OBJ_FLAG_SCROLLABLE);

    app->city_label = lv_label_create(app->status_bar);
    lv_label_set_text(app->city_label, app->model.current.city);
    lv_obj_set_style_text_color(app->city_label, WD_COLOR_TEXT, 0);
    lv_obj_set_style_text_font(app->city_label, &lv_font_montserrat_14, 0);
    lv_obj_align(app->city_label, LV_ALIGN_LEFT_MID, 0, 0);

    app->time_label = lv_label_create(app->status_bar);
    lv_label_set_text(app->time_label, "00:00");
    lv_obj_set_style_text_color(app->time_label, WD_COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(app->time_label, &lv_font_montserrat_14, 0);
    lv_obj_align(app->time_label, LV_ALIGN_RIGHT_MID, 0, 0);

    /* ---- Content area ---- */
    app->content = lv_obj_create(app->scr);
    lv_obj_set_size(app->content, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(app->content, 1);
    lv_obj_set_style_bg_opa(app->content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(app->content, 0, 0);
    lv_obj_set_style_pad_all(app->content, 0, 0);

    /* ---- Bottom navigation ---- */
    app->navbar = lv_obj_create(app->scr);
    lv_obj_set_size(app->navbar, lv_pct(100), 52);
    lv_obj_set_style_bg_color(app->navbar, WD_COLOR_CARD, 0);
    lv_obj_set_style_bg_opa(app->navbar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(app->navbar, 0, 0);
    lv_obj_set_style_border_width(app->navbar, 0, 0);
    lv_obj_set_style_border_side(app->navbar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_color(app->navbar, WD_COLOR_DIVIDER, 0);
    lv_obj_set_style_border_width(app->navbar, 1, 0);
    lv_obj_set_flex_flow(app->navbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(app->navbar, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(app->navbar, LV_OBJ_FLAG_SCROLLABLE);

    int i;
    for (i = 0; i < WD_PAGE_MAX; i++) {
        app->nav_btns[i] = lv_btn_create(app->navbar);
        lv_obj_set_size(app->nav_btns[i], 70, 44);
        lv_obj_set_style_bg_opa(app->nav_btns[i], LV_OPA_TRANSP, 0);
        lv_obj_set_style_shadow_width(app->nav_btns[i], 0, 0);
        lv_obj_set_flex_flow(app->nav_btns[i], LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(app->nav_btns[i], LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(app->nav_btns[i], 2, 0);
        lv_obj_add_event_cb(app->nav_btns[i], nav_btn_cb,
                            LV_EVENT_CLICKED, app);

        app->nav_labels[i] = lv_label_create(app->nav_btns[i]);
        char buf[32];
        snprintf(buf, sizeof(buf), "%s\n%s", g_nav_icons[i], g_nav_labels[i]);
        lv_label_set_text(app->nav_labels[i], buf);
        lv_obj_set_style_text_align(app->nav_labels[i],
                                    LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(app->nav_labels[i],
                                   &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(app->nav_labels[i],
                                    (i == 0) ? WD_COLOR_PRIMARY
                                             : WD_COLOR_TEXT_DIM, 0);
    }

    /* ---- Timers ---- */
    app->data_timer  = lv_timer_create(data_timer_cb, 5000, app);
    app->clock_timer = lv_timer_create(clock_timer_cb, 1000, app);

    /* ---- Show home page ---- */
    app->current_page = WD_PAGE_HOME;
    lv_obj_t *page_obj = lv_obj_create(app->content);
    lv_obj_set_size(page_obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(page_obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page_obj, 0, 0);
    lv_obj_set_style_pad_all(page_obj, 0, 0);
    app->page_objs[WD_PAGE_HOME] = page_obj;
    page_home_create(app, page_obj);
}

static void app_destroy(weather_app_t *app)
{
    if (app->data_timer)  lv_timer_delete(app->data_timer);
    if (app->clock_timer) lv_timer_delete(app->clock_timer);

    if (g_page_ops[app->current_page].destroy) {
        g_page_ops[app->current_page].destroy(app);
    }
}

/* ---- UV loop ---- */

static void lv_nuttx_uv_loop(uv_loop_t *loop, lv_nuttx_result_t *result)
{
    lv_nuttx_uv_t uv_info;
    void *data;

    uv_loop_init(loop);

    lv_memset(&uv_info, 0, sizeof(uv_info));
    uv_info.loop  = loop;
    uv_info.disp  = result->disp;
    uv_info.indev = result->indev;
#ifdef CONFIG_UINPUT_TOUCH
    uv_info.uindev = result->utouch_indev;
#endif

    data = lv_nuttx_uv_init(&uv_info);
    uv_run(loop, UV_RUN_DEFAULT);
    lv_nuttx_uv_deinit(&data);
}

/* ---- Entry point ---- */

int weather_dashboard_main(int argc, FAR char *argv[])
{
    lv_nuttx_dsc_t    info;
    lv_nuttx_result_t result;
    uv_loop_t         ui_loop;

    lv_memset(&ui_loop, 0, sizeof(uv_loop_t));

    if (lv_is_initialized()) {
        LV_LOG_ERROR("LVGL already initialized! aborting.");
        return -1;
    }

    lv_init();
    lv_nuttx_dsc_init(&info);
    lv_nuttx_init(&info, &result);

    if (result.disp == NULL) {
        LV_LOG_ERROR("weather_dashboard: display init failure!");
        return 1;
    }

    app_create(&g_app);

    lv_nuttx_uv_loop(&ui_loop, &result);

    app_destroy(&g_app);
    lv_nuttx_deinit(&result);
    lv_deinit();

    return 0;
}
