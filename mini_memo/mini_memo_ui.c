/****************************************************************************
 * Copyright (C) 2026 Xiaomi Corporation
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
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include <lvgl/lvgl.h>

#include "mini_memo_core.h"
#include "mini_memo_ui.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define UI_BG_COLOR 0x121220
#define UI_ACCENT_COLOR 0x3a7bd5
#define UI_TEXT_MUTED 0x888899
#define UI_CARD_BG 0x1e1e30
#define MEMO_MAX_DISPLAY 20

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int g_screen_w = 320;
static int g_screen_h = 240;

static lv_obj_t* g_tileview;
static lv_obj_t* g_tiles[4]; /* Home, Voice, Review, Settings */
static lv_obj_t* g_nav_dots[4];
static lv_obj_t* g_nav_bar;
static lv_obj_t* g_home_memo_count;
static lv_obj_t* g_home_todo_count;
static lv_obj_t* g_home_sched_count;
static lv_timer_t* g_flush_timer;
static lv_timer_t* g_remind_timer;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static const lv_font_t* get_font_large(void)
{
#if LV_FONT_MONTSERRAT_30
    return &lv_font_montserrat_30;
#else
    return LV_FONT_DEFAULT;
#endif
}

static const lv_font_t* get_font_medium(void)
{
#if LV_FONT_MONTSERRAT_20
    return &lv_font_montserrat_20;
#else
    return LV_FONT_DEFAULT;
#endif
}

static const lv_font_t* get_font_small(void)
{
#if LV_FONT_MONTSERRAT_16
    return &lv_font_montserrat_16;
#else
    return LV_FONT_DEFAULT;
#endif
}

/* -- Periodic flush timer ------------------------------- */

static void flush_timer_cb(lv_timer_t* timer)
{
    (void)timer;
    memo_store_flush();
}

/* -- Reminder check timer ------------------------------ */

static void remind_timer_cb(lv_timer_t* timer)
{
    memo_item_t items[MEMO_MAX_DISPLAY];
    int count;
    int i;
    int64_t now;

    (void)timer;
    now = (int64_t)time(NULL);

    count = memo_store_get_due_reminders(now, items, MEMO_MAX_DISPLAY);
    for (i = 0; i < count; i++) {
        memo_ui_show_notification("Reminder", items[i].content);
        memo_store_mark_read(items[i].id);
    }
}

/* -- Navigation dots ----------------------------------- */

static void update_nav_dots(int active_idx)
{
    int i;

    for (i = 0; i < 4; i++) {
        if (g_nav_dots[i] == NULL) {
            continue;
        }
        if (i == active_idx) {
            lv_obj_set_style_bg_color(g_nav_dots[i],
                lv_color_hex(UI_ACCENT_COLOR), 0);
            lv_obj_set_size(g_nav_dots[i], 20, 8);
        } else {
            lv_obj_set_style_bg_color(g_nav_dots[i],
                lv_color_hex(UI_TEXT_MUTED), 0);
            lv_obj_set_size(g_nav_dots[i], 8, 8);
        }
    }
}

static void tileview_changed_cb(lv_event_t* e)
{
    lv_obj_t* tv = lv_event_get_target(e);
    lv_obj_t* tile = lv_tileview_get_tile_active(tv);
    int i;

    for (i = 0; i < 4; i++) {
        if (g_tiles[i] == tile) {
            update_nav_dots(i);
            break;
        }
    }
}

static void create_nav_bar(lv_obj_t* parent)
{
    int i;

    g_nav_bar = lv_obj_create(parent);
    lv_obj_set_size(g_nav_bar, g_screen_w, 30);
    lv_obj_align(g_nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(g_nav_bar, lv_color_hex(UI_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(g_nav_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_nav_bar, 0, 0);
    lv_obj_set_style_pad_all(g_nav_bar, 0, 0);
    lv_obj_set_flex_flow(g_nav_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(g_nav_bar, LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(g_nav_bar, 8, 0);
    lv_obj_clear_flag(g_nav_bar, LV_OBJ_FLAG_SCROLLABLE);

    for (i = 0; i < 4; i++) {
        g_nav_dots[i] = lv_obj_create(g_nav_bar);
        lv_obj_set_size(g_nav_dots[i], 8, 8);
        lv_obj_set_style_radius(g_nav_dots[i], 4, 0);
        lv_obj_set_style_bg_color(g_nav_dots[i],
            lv_color_hex(UI_TEXT_MUTED), 0);
        lv_obj_set_style_bg_opa(g_nav_dots[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(g_nav_dots[i], 0, 0);
        lv_obj_clear_flag(g_nav_dots[i], LV_OBJ_FLAG_SCROLLABLE);
    }

    update_nav_dots(0);
}

/* -- Page: Home ---------------------------------------- */

static lv_obj_t* create_stat_card(lv_obj_t* parent,
    const char* icon, const char* label, int count)
{
    char buf[32];
    lv_obj_t* card;
    lv_obj_t* lbl_icon;
    lv_obj_t* lbl_count;
    lv_obj_t* lbl_name;

    card = lv_obj_create(parent);
    lv_obj_set_size(card, (g_screen_w - 80) / 3, 100);
    lv_obj_set_style_bg_color(card, lv_color_hex(UI_CARD_BG), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 16, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lbl_icon = lv_label_create(card);
    lv_label_set_text(lbl_icon, icon);
    lv_obj_set_style_text_font(lbl_icon, get_font_large(), 0);
    lv_obj_set_style_text_color(lbl_icon, lv_color_hex(UI_ACCENT_COLOR), 0);

    snprintf(buf, sizeof(buf), "%d", count);
    lbl_count = lv_label_create(card);
    lv_label_set_text(lbl_count, buf);
    lv_obj_set_style_text_font(lbl_count, get_font_large(), 0);
    lv_obj_set_style_text_color(lbl_count, lv_color_white(), 0);

    lbl_name = lv_label_create(card);
    lv_label_set_text(lbl_name, label);
    lv_obj_set_style_text_font(lbl_name, get_font_small(), 0);
    lv_obj_set_style_text_color(lbl_name, lv_color_hex(UI_TEXT_MUTED), 0);

    return lbl_count;
}

static void create_home_page(lv_obj_t* tile)
{
    lv_obj_t* title;
    lv_obj_t* card_row;
    int memo_cnt;
    int todo_cnt;
    int sched_cnt;

    lv_obj_set_style_pad_all(tile, 30, 0);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(tile, 20, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    /* Title */

    title = lv_label_create(tile);
    lv_label_set_text(title, LV_SYMBOL_HOME "  Mini Memo");
    lv_obj_set_style_text_font(title, get_font_large(), 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);

    /* Stats row */

    card_row = lv_obj_create(tile);
    lv_obj_set_size(card_row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(card_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(card_row, 0, 0);
    lv_obj_set_style_pad_all(card_row, 0, 0);
    lv_obj_set_flex_flow(card_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card_row, LV_FLEX_ALIGN_SPACE_EVENLY,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(card_row, LV_OBJ_FLAG_SCROLLABLE);

    memo_cnt = memo_store_get_count(MEMO_TYPE_MEMO, true);
    todo_cnt = memo_store_get_count(MEMO_TYPE_TODO, true);
    sched_cnt = memo_store_get_count(MEMO_TYPE_SCHEDULE, true);

    g_home_memo_count = create_stat_card(card_row,
        LV_SYMBOL_FILE, "Memos", memo_cnt);
    g_home_todo_count = create_stat_card(card_row,
        LV_SYMBOL_LIST, "Todos", todo_cnt);
    g_home_sched_count = create_stat_card(card_row,
        LV_SYMBOL_BELL, "Schedule", sched_cnt);
}

/* -- Page: Voice --------------------------------------- */

static lv_obj_t* g_voice_status_lbl;
static lv_obj_t* g_ptt_btn;
static bool g_recording;
static lv_obj_t* g_review_list;
static lv_timer_t* g_ptt_selftest_start_timer;
static lv_timer_t* g_ptt_selftest_release_timer;
static uint32_t g_ptt_selftest_hold_ms;

static void refresh_review_list(void);
static void ptt_selftest_release_cb(lv_timer_t* timer);

static void ptt_selftest_start_cb(lv_timer_t* timer)
{
    lv_area_t coords;

    if (timer != NULL) {
        lv_timer_delete(timer);
        g_ptt_selftest_start_timer = NULL;
    }

    if (g_tileview == NULL || g_tiles[MEMO_PAGE_VOICE] == NULL || g_ptt_btn == NULL) {
        syslog(LOG_ERR, "PTT selftest: UI not ready\n");
        return;
    }

    lv_tileview_set_tile(g_tileview, g_tiles[MEMO_PAGE_VOICE], LV_ANIM_OFF);
    update_nav_dots(MEMO_PAGE_VOICE);
    lv_obj_update_layout(lv_screen_active());
    lv_obj_get_coords(g_ptt_btn, &coords);

    syslog(LOG_INFO,
        "PTT selftest: voice page ready, btn=(%ld,%ld)-(%ld,%ld), hold=%" PRIu32 "ms\n",
        (long)coords.x1, (long)coords.y1, (long)coords.x2, (long)coords.y2,
        g_ptt_selftest_hold_ms);

    lv_obj_send_event(g_ptt_btn, LV_EVENT_PRESSED, NULL);

    g_ptt_selftest_release_timer = lv_timer_create(ptt_selftest_release_cb,
        g_ptt_selftest_hold_ms, NULL);
    if (g_ptt_selftest_release_timer != NULL) {
        lv_timer_set_repeat_count(g_ptt_selftest_release_timer, 1);
    }
}

static void ptt_selftest_release_cb(lv_timer_t* timer)
{
    if (timer != NULL) {
        lv_timer_delete(timer);
        g_ptt_selftest_release_timer = NULL;
    }

    if (g_ptt_btn == NULL) {
        syslog(LOG_ERR, "PTT selftest: button missing on release\n");
        return;
    }

    syslog(LOG_INFO, "PTT selftest: releasing button\n");
    lv_obj_send_event(g_ptt_btn, LV_EVENT_RELEASED, NULL);
}

/* Payload for marshalling classify results onto the LVGL thread. */

typedef struct {
    char status_text[64];
    bool refresh;
} classify_ui_t;

/* Runs on the LVGL thread (via lv_async_call) where lv_* is safe. */

static void classify_done_async_cb(void* p)
{
    classify_ui_t* u = (classify_ui_t*)p;

    if (g_voice_status_lbl) {
        lv_label_set_text(g_voice_status_lbl, u->status_text);
    }

    if (u->refresh) {
        memo_ui_refresh_home();
        refresh_review_list();
    }

    free(u);
}

static void on_classify_done(int status, const classify_result_t* result,
    void* cookie)
{
    memo_item_t item;
    classify_ui_t* u;
    const char* type_name = "Memo";
    (void)cookie;

    /* This callback may run on the velaclaw SDK worker thread; lv_* is not
     * thread-safe, so all UI work is deferred to the LVGL thread. */

    u = malloc(sizeof(*u));
    if (!u) {
        syslog(LOG_ERR, "classify: ui payload alloc failed\n");
        return;
    }

    if (status != 0 || !result) {
        syslog(LOG_ERR, "classify failed status=%d\n", status);
        strncpy(u->status_text, "Classification failed",
            sizeof(u->status_text) - 1);
        u->status_text[sizeof(u->status_text) - 1] = '\0';
        u->refresh = false;
        lv_async_call(classify_done_async_cb, u);
        return;
    }

    /* Store the new memo (memo_store is thread-safe via internal mutex) */

    memset(&item, 0, sizeof(item));
    item.type = result->type;
    strncpy(item.content, result->content, sizeof(item.content) - 1);
    item.content[sizeof(item.content) - 1] = '\0';
    item.timestamp = time(NULL);
    item.remind_at = result->remind_at;
    item.is_read = false;

    memo_store_add(&item);

    if (result->type == MEMO_TYPE_TODO) {
        type_name = "Todo";
    } else if (result->type == MEMO_TYPE_SCHEDULE) {
        type_name = "Schedule";
    }

    snprintf(u->status_text, sizeof(u->status_text), "Saved as %s!",
        type_name);
    u->refresh = true;

    lv_async_call(classify_done_async_cb, u);
}

static void ptt_pressed_cb(lv_event_t* e)
{
    (void)e;
    int ret;

    if (g_recording) {
        return;
    }

    syslog(LOG_INFO, "PTT: pressed - start recording\n");
    g_recording = true;

    lv_obj_set_style_bg_color(g_ptt_btn, lv_color_hex(0xd53a3a), 0);
    lv_label_set_text(g_voice_status_lbl, "Recording...");

    ret = memo_voice_start();
    if (ret < 0) {
        syslog(LOG_ERR, "PTT: voice_start failed: %d\n", ret);
        lv_label_set_text(g_voice_status_lbl, "Mic unavailable");
        lv_obj_set_style_bg_color(g_ptt_btn,
            lv_color_hex(UI_ACCENT_COLOR), 0);
        g_recording = false;
    }
}

static void ptt_released_cb(lv_event_t* e)
{
    (void)e;
    char text_buf[512];
    int ret;

    if (!g_recording) {
        return;
    }

    syslog(LOG_INFO, "PTT: released - stop recording\n");
    g_recording = false;

    lv_obj_set_style_bg_color(g_ptt_btn, lv_color_hex(UI_ACCENT_COLOR), 0);
    lv_label_set_text(g_voice_status_lbl, "Recognizing...");

    ret = memo_voice_stop(text_buf, sizeof(text_buf));
    if (ret < 0 || text_buf[0] == '\0') {
        lv_label_set_text(g_voice_status_lbl,
            ret < 0 ? "ASR failed" : "No speech detected");
        return;
    }

    /* Show transcribed text */

    lv_label_set_text(g_voice_status_lbl, text_buf);

    /* Classify and store */

    memo_classify_async(text_buf, on_classify_done, NULL);
}

static void create_voice_page(lv_obj_t* tile)
{
    lv_obj_t* title;
    lv_obj_t* ptt_lbl;

    lv_obj_set_style_pad_all(tile, 30, 0);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(tile, 20, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    title = lv_label_create(tile);
    lv_label_set_text(title, LV_SYMBOL_AUDIO "  Voice Memo");
    lv_obj_set_style_text_font(title, get_font_large(), 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);

    /* PTT button (circular) */

    g_ptt_btn = lv_obj_create(tile);
    lv_obj_set_size(g_ptt_btn, 80, 80);
    lv_obj_set_style_radius(g_ptt_btn, 40, 0);
    lv_obj_set_style_bg_color(g_ptt_btn, lv_color_hex(UI_ACCENT_COLOR), 0);
    lv_obj_set_style_bg_opa(g_ptt_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_ptt_btn, 0, 0);
    lv_obj_clear_flag(g_ptt_btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_ptt_btn, LV_OBJ_FLAG_CLICKABLE);

    ptt_lbl = lv_label_create(g_ptt_btn);
    lv_label_set_text(ptt_lbl, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_font(ptt_lbl, get_font_large(), 0);
    lv_obj_set_style_text_color(ptt_lbl, lv_color_white(), 0);
    lv_obj_center(ptt_lbl);

    /* PTT event handlers */

    lv_obj_add_event_cb(g_ptt_btn, ptt_pressed_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g_ptt_btn, ptt_released_cb, LV_EVENT_RELEASED, NULL);

    /* Status label */

    g_voice_status_lbl = lv_label_create(tile);
    lv_label_set_text(g_voice_status_lbl, "Hold button to record");
    lv_obj_set_style_text_font(g_voice_status_lbl, get_font_small(), 0);
    lv_obj_set_style_text_color(g_voice_status_lbl,
        lv_color_hex(UI_TEXT_MUTED), 0);
    lv_obj_set_width(g_voice_status_lbl, LV_PCT(90));
    lv_label_set_long_mode(g_voice_status_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(g_voice_status_lbl, LV_TEXT_ALIGN_CENTER, 0);

    g_recording = false;
}

/* -- Page: Review -------------------------------------- */

static void review_item_swipe_cb(lv_event_t* e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    lv_obj_t* row;
    uint32_t id;

    if (dir != LV_DIR_LEFT) {
        return;
    }

    row = lv_event_get_current_target(e);
    id = (uint32_t)(uintptr_t)lv_event_get_user_data(e);

    memo_store_delete(id);

    /* Animate row out then refresh */

    lv_obj_add_flag(row, LV_OBJ_FLAG_HIDDEN);
    refresh_review_list();
    memo_ui_refresh_home();
}

static void refresh_review_list(void)
{
    memo_item_t items[MEMO_MAX_DISPLAY];
    int count;
    int i;

    if (!g_review_list) {
        return;
    }

    lv_obj_clean(g_review_list);

    count = memo_store_get_recent(items, MEMO_MAX_DISPLAY);
    if (count == 0) {
        lv_obj_t* empty = lv_label_create(g_review_list);
        lv_label_set_text(empty,
            "No memos yet.\nSwipe to Voice page to add one.");
        lv_obj_set_style_text_color(empty, lv_color_hex(UI_TEXT_MUTED), 0);
        lv_obj_set_style_text_font(empty, get_font_small(), 0);
        return;
    }

    /* Show items newest-first */

    for (i = count - 1; i >= 0; i--) {
        lv_obj_t* row = lv_obj_create(g_review_list);
        lv_obj_t* type_lbl;
        lv_obj_t* content_lbl;
        const char* type_icon;

        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 4, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(row, 10, 0);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_GESTURE_BUBBLE);

        /* Swipe left to delete */

        lv_obj_add_event_cb(row, review_item_swipe_cb,
            LV_EVENT_GESTURE,
            (void*)(uintptr_t)items[i].id);

        switch (items[i].type) {
        case MEMO_TYPE_TODO:
            type_icon = LV_SYMBOL_LIST;
            break;
        case MEMO_TYPE_SCHEDULE:
            type_icon = LV_SYMBOL_BELL;
            break;
        default:
            type_icon = LV_SYMBOL_FILE;
            break;
        }

        type_lbl = lv_label_create(row);
        lv_label_set_text(type_lbl, type_icon);
        lv_obj_set_style_text_color(type_lbl,
            lv_color_hex(UI_ACCENT_COLOR), 0);

        content_lbl = lv_label_create(row);
        lv_label_set_text(content_lbl, items[i].content);
        lv_obj_set_style_text_color(content_lbl, lv_color_white(), 0);
        lv_obj_set_style_text_font(content_lbl, get_font_small(), 0);
        lv_obj_set_width(content_lbl, LV_PCT(90));
        lv_label_set_long_mode(content_lbl, LV_LABEL_LONG_DOT);
    }
}

static void create_review_page(lv_obj_t* tile)
{
    lv_obj_t* title;
    lv_obj_t* list;

    lv_obj_set_style_pad_all(tile, 30, 0);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(tile, 10, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    title = lv_label_create(tile);
    lv_label_set_text(title, LV_SYMBOL_LIST "  Review");
    lv_obj_set_style_text_font(title, get_font_large(), 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);

    /* Scrollable list of memos */

    list = lv_obj_create(tile);
    lv_obj_set_size(list, LV_PCT(100), g_screen_h - 130);
    lv_obj_set_style_bg_color(list, lv_color_hex(UI_CARD_BG), 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(list, 12, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 16, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list, 8, 0);

    g_review_list = list;
    refresh_review_list();
}

/* -- Page: Settings ------------------------------------ */

static void create_settings_page(lv_obj_t* tile)
{
    lv_obj_t* title;
    lv_obj_t* info;

    lv_obj_set_style_pad_all(tile, 30, 0);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(tile, 20, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    title = lv_label_create(tile);
    lv_label_set_text(title, LV_SYMBOL_SETTINGS "  Settings");
    lv_obj_set_style_text_font(title, get_font_large(), 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);

    info = lv_label_create(tile);
    lv_label_set_text(info,
        "Review Interval: 4 hours\n\n"
        "Storage: /data/mini_memo\n\n"
        "Max Items: 100\n\n"
        "Version: 1.0.0");
    lv_obj_set_style_text_font(info, get_font_medium(), 0);
    lv_obj_set_style_text_color(info, lv_color_hex(UI_TEXT_MUTED), 0);
    lv_obj_set_style_text_line_space(info, 6, 0);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int memo_ui_init(void)
{
    lv_obj_t* scr = lv_scr_act();
    lv_display_t* disp = lv_display_get_default();

    syslog(LOG_INFO, "memo_ui_init: creating 4-page UI\n");

    /* Get actual display resolution */

    if (disp) {
        g_screen_w = lv_display_get_horizontal_resolution(disp);
        g_screen_h = lv_display_get_vertical_resolution(disp);
    }

    syslog(LOG_INFO, "memo_ui_init: using res=%dx%d\n", g_screen_w, g_screen_h);

    /* Dark background for screen */

    lv_obj_set_style_bg_color(scr, lv_color_hex(UI_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Create tileview (horizontal swipe between 4 pages) */

    g_tileview = lv_tileview_create(scr);
    lv_obj_set_size(g_tileview, g_screen_w, g_screen_h - 30);
    lv_obj_align(g_tileview, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(g_tileview, LV_OPA_TRANSP, 0);
    lv_obj_set_scroll_snap_x(g_tileview, LV_SCROLL_SNAP_CENTER);

    /* Add tiles: col 0-3, row 0 (horizontal only) */

    g_tiles[MEMO_PAGE_HOME] = lv_tileview_add_tile(
        g_tileview, 0, 0, LV_DIR_RIGHT);
    g_tiles[MEMO_PAGE_VOICE] = lv_tileview_add_tile(
        g_tileview, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    g_tiles[MEMO_PAGE_REVIEW] = lv_tileview_add_tile(
        g_tileview, 2, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    g_tiles[MEMO_PAGE_SETTINGS] = lv_tileview_add_tile(
        g_tileview, 3, 0, LV_DIR_LEFT);

    /* Populate each page */

    create_home_page(g_tiles[MEMO_PAGE_HOME]);
    create_voice_page(g_tiles[MEMO_PAGE_VOICE]);
    create_review_page(g_tiles[MEMO_PAGE_REVIEW]);
    create_settings_page(g_tiles[MEMO_PAGE_SETTINGS]);

    /* Ensure tiles don't eat horizontal scroll - tileview owns it */

    {
        int i;
        for (i = 0; i < 4; i++) {
            lv_obj_add_flag(g_tiles[i], LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
            lv_obj_clear_flag(g_tiles[i], LV_OBJ_FLAG_SCROLL_ELASTIC);
        }
    }

    /* Navigation dots at bottom */

    create_nav_bar(scr);

    /* Listen for tile changes to update dots */

    lv_obj_add_event_cb(g_tileview, tileview_changed_cb,
        LV_EVENT_VALUE_CHANGED, NULL);

    /* Periodic flush timer (5 seconds) */

    g_flush_timer = lv_timer_create(flush_timer_cb, 5000, NULL);

    /* Reminder check timer (60 seconds) */

    g_remind_timer = lv_timer_create(remind_timer_cb, 60000, NULL);

    return 0;
}

void memo_ui_deinit(void)
{
    syslog(LOG_INFO, "memo_ui_deinit\n");
    if (g_ptt_selftest_start_timer) {
        lv_timer_delete(g_ptt_selftest_start_timer);
        g_ptt_selftest_start_timer = NULL;
    }
    if (g_ptt_selftest_release_timer) {
        lv_timer_delete(g_ptt_selftest_release_timer);
        g_ptt_selftest_release_timer = NULL;
    }
    if (g_flush_timer) {
        lv_timer_delete(g_flush_timer);
        g_flush_timer = NULL;
    }
    if (g_remind_timer) {
        lv_timer_delete(g_remind_timer);
        g_remind_timer = NULL;
    }
    g_tileview = NULL;
}

void memo_ui_show_notification(const char* title, const char* body)
{
    lv_obj_t* mbox;

    syslog(LOG_INFO, "memo_ui_show_notification: %s - %s\n", title, body);

    mbox = lv_msgbox_create(NULL);
    lv_msgbox_add_title(mbox, title);
    lv_msgbox_add_text(mbox, body);
    lv_msgbox_add_close_button(mbox);
    lv_obj_center(mbox);
}

void memo_ui_refresh_home(void)
{
    char buf[16];
    int count;

    if (g_home_memo_count == NULL) {
        return;
    }

    count = memo_store_get_count(MEMO_TYPE_MEMO, true);
    snprintf(buf, sizeof(buf), "%d", count);
    lv_label_set_text(g_home_memo_count, buf);

    count = memo_store_get_count(MEMO_TYPE_TODO, true);
    snprintf(buf, sizeof(buf), "%d", count);
    lv_label_set_text(g_home_todo_count, buf);

    count = memo_store_get_count(MEMO_TYPE_SCHEDULE, true);
    snprintf(buf, sizeof(buf), "%d", count);
    lv_label_set_text(g_home_sched_count, buf);
}

void memo_ui_navigate_to(int page_index)
{
    if (page_index < 0 || page_index > 3) {
        return;
    }

    if (g_tileview == NULL || g_tiles[page_index] == NULL) {
        return;
    }

    lv_tileview_set_tile(g_tileview, g_tiles[page_index], LV_ANIM_ON);
    update_nav_dots(page_index);
}

int memo_ui_start_ptt_selftest(unsigned int hold_ms)
{
    if (g_ptt_btn == NULL || g_tileview == NULL || g_tiles[MEMO_PAGE_VOICE] == NULL) {
        return -ENODEV;
    }

    if (g_ptt_selftest_start_timer != NULL || g_ptt_selftest_release_timer != NULL) {
        return -EBUSY;
    }

    g_ptt_selftest_hold_ms = hold_ms == 0 ? 1000 : hold_ms;
    g_ptt_selftest_start_timer = lv_timer_create(ptt_selftest_start_cb, 400, NULL);
    if (g_ptt_selftest_start_timer == NULL) {
        return -ENOMEM;
    }

    lv_timer_set_repeat_count(g_ptt_selftest_start_timer, 1);
    syslog(LOG_INFO, "PTT selftest: scheduled\n");
    return 0;
}
