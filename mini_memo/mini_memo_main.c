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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#ifndef CONFIG_LV_USE_NUTTX_LIBUV
#include <unistd.h>
#else
#include <uv.h>
#endif

#include <lvgl/lvgl.h>

#include "mini_memo_core.h"
#include "mini_memo_ui.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_MINI_MEMO_DATA_DIR
#define CONFIG_MINI_MEMO_DATA_DIR "/data/mini_memo"
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
static void lv_nuttx_uv_loop(uv_loop_t* loop, lv_nuttx_result_t* result)
{
    lv_nuttx_uv_t uv_info;
    void* data;

    uv_loop_init(loop);

    lv_memset(&uv_info, 0, sizeof(uv_info));
    uv_info.loop = loop;
    uv_info.disp = result->disp;
    uv_info.indev = result->indev;
#ifdef CONFIG_UINPUT_TOUCH
    uv_info.uindev = result->utouch_indev;
#endif

    data = lv_nuttx_uv_init(&uv_info);
    uv_run(loop, UV_RUN_DEFAULT);
    lv_nuttx_uv_deinit(&data);
}
#else
static void lv_nuttx_loop(void)
{
    while (1) {
        uint32_t idle;

        idle = lv_timer_handler();
        idle = idle ? idle : 1;
        usleep(idle * 1000);
    }
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char* argv[])
{
    lv_nuttx_dsc_t info;
    lv_nuttx_result_t result;
#ifdef CONFIG_LV_USE_NUTTX_LIBUV
    uv_loop_t ui_loop;
#endif
    char selftest_flag_path[128];
    bool ptt_selftest = false;
    int ret;
    int i;

    syslog(LOG_INFO, "Mini Memo starting\n");

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ptt-selftest") == 0 || strcmp(argv[i], "ptt-selftest") == 0 || strcmp(argv[i], "selftest") == 0) {
            ptt_selftest = true;
            syslog(LOG_INFO, "Mini Memo: PTT selftest requested via '%s'\n",
                argv[i]);
        }
    }

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
    lv_memset(&ui_loop, 0, sizeof(uv_loop_t));
#endif

    if (lv_is_initialized()) {
        syslog(LOG_ERR, "Mini Memo: LVGL already initialized\n");
        return -1;
    }

    lv_init();

    lv_nuttx_dsc_init(&info);

    /* Match luncher_mini initialization exactly */

#ifdef CONFIG_LV_USE_NUTTX_LCD
    info.fb_path = "/dev/lcd0";
#endif

    lv_nuttx_init(&info, &result);

    if (result.disp == NULL) {
        syslog(LOG_ERR, "Mini Memo: display init failed\n");
        lv_deinit();
        return -1;
    }

    ret = memo_store_init(CONFIG_MINI_MEMO_DATA_DIR);
    if (ret < 0) {
        syslog(LOG_ERR, "Mini Memo: store init failed: %d\n", ret);
        lv_nuttx_deinit(&result);
        lv_deinit();
        return ret;
    }

    snprintf(selftest_flag_path, sizeof(selftest_flag_path),
        "%s/.ptt_selftest", CONFIG_MINI_MEMO_DATA_DIR);
    if (!ptt_selftest && access(selftest_flag_path, F_OK) == 0) {
        ptt_selftest = true;
        syslog(LOG_INFO, "Mini Memo: PTT selftest requested via '%s'\n",
            selftest_flag_path);
    }

    /* Initialize AI agent (voice + LLM). Non-fatal if it fails. */

    ret = memo_agent_init();
    if (ret < 0) {
        syslog(LOG_WARNING, "Mini Memo: voice init failed: %d\n", ret);
    }

    ret = memo_ui_init();
    if (ret < 0) {
        syslog(LOG_ERR, "Mini Memo: UI init failed: %d\n", ret);
        memo_store_deinit();
        lv_nuttx_deinit(&result);
        lv_deinit();
        return ret;
    }

    if (ptt_selftest) {
        ret = memo_ui_start_ptt_selftest(1500);
        if (ret < 0) {
            syslog(LOG_ERR, "Mini Memo: PTT selftest scheduling failed: %d\n", ret);
        }
    }

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
    syslog(LOG_INFO, "Mini Memo: entering libuv event loop\n");
    lv_nuttx_uv_loop(&ui_loop, &result);
    syslog(LOG_INFO, "Mini Memo: libuv loop exited\n");
#else
    syslog(LOG_INFO, "Mini Memo: entering poll loop, indev=%p disp=%p\n",
        result.indev, result.disp);
    syslog(LOG_INFO, "Mini Memo: display res=%ldx%ld\n",
        (long)lv_display_get_horizontal_resolution(result.disp),
        (long)lv_display_get_vertical_resolution(result.disp));

    /* Ensure indev is in timer (poll) mode with 30ms period */
    if (result.indev) {
        lv_indev_set_mode(result.indev, LV_INDEV_MODE_TIMER);
        lv_timer_t* t = lv_indev_get_read_timer(result.indev);
        if (t) {
            lv_timer_set_period(t, 30);
            syslog(LOG_INFO, "Mini Memo: indev timer set to 30ms\n");
        } else {
            syslog(LOG_ERR, "Mini Memo: indev has no timer!\n");
        }
    } else {
        syslog(LOG_ERR, "Mini Memo: indev is NULL!\n");
    }

    lv_nuttx_loop();
#endif

    memo_ui_deinit();
    memo_agent_deinit();
    memo_store_deinit();
    lv_nuttx_deinit(&result);
    lv_deinit();

    syslog(LOG_INFO, "Mini Memo exiting\n");
    return 0;
}
