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
#include "stopwatch_ui.h"

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

int stopwatch_main(int argc, FAR char *argv[])
{
    lv_nuttx_dsc_t    info;
    lv_nuttx_result_t result;
    uv_loop_t         ui_loop;
    stopwatch_ui_t   *ui = NULL;

    lv_memset(&ui_loop, 0, sizeof(uv_loop_t));

    if (lv_is_initialized()) {
        LV_LOG_ERROR("LVGL already initialized! aborting.");
        return -1;
    }

    lv_init();
    lv_nuttx_dsc_init(&info);
    lv_nuttx_init(&info, &result);

    if (result.disp == NULL) {
        LV_LOG_ERROR("stopwatch: display initialization failure!");
        return 1;
    }

    /* Create the stopwatch UI */

    lv_obj_t *scr = lv_screen_active();
    ui = stopwatch_ui_create(scr);

    if (!ui) {
        LV_LOG_ERROR("stopwatch: UI creation failure!");
        lv_nuttx_deinit(&result);
        lv_deinit();
        return 1;
    }

    /* Enter the event loop */

    lv_nuttx_uv_loop(&ui_loop, &result);

    /* Cleanup */

    stopwatch_ui_destroy(ui);
    lv_nuttx_deinit(&result);
    lv_deinit();

    return 0;
}
