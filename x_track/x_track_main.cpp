/*
 * MIT License
 * Copyright (c) 2024 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "App/App.h"
#include "HAL/HAL.h"
#include "lvgl/lvgl.h"
#include <uv.h>

static void lv_nuttx_uv_loop_run(uv_loop_t* loop, lv_nuttx_result_t* result)
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

/**
 * @brief  Main Function
 * @param  argc: Argument count
 * @param  argv: Argument vector
 * @retval error code
 */
extern "C" int main(int argc, const char* argv[])
{
    lv_nuttx_dsc_t info;
    lv_nuttx_result_t result;
    uv_loop_t ui_loop;
    lv_memzero(&ui_loop, sizeof(uv_loop_t));

    if (lv_is_initialized()) {
        LV_LOG_ERROR("LVGL already initialized! aborting.");
        return -1;
    }

    lv_init();
    lv_nuttx_dsc_init(&info);
    lv_nuttx_init(&info, &result);

    if (result.disp == NULL) {
        LV_LOG_ERROR("Display initialization failure!");
        return 1;
    }

    HAL::Init();

    AppContext_t* appCtx = App_CreateContext(argc, argv);
    LV_LOG_USER("App context created: %p", appCtx);

    /* Create app timer */
    lv_timer_t* app_timer = lv_timer_create(
        [](lv_timer_t* tmr) {
            auto ctx = (AppContext_t*)lv_timer_get_user_data(tmr);

            /* Run app loop */
            uint32_t app_idle_time = App_RunLoopExecute(ctx);

            lv_timer_set_period(tmr, LV_MIN(app_idle_time, 1000));
        },
        0,
        appCtx);

    /* Run UI loop */
    lv_nuttx_uv_loop_run(&ui_loop, &result);

    /* Clean up */
    lv_timer_delete(app_timer);
    App_DestroyContext(appCtx);
    lv_display_delete(result.disp);
    lv_deinit();
    return 0;
}
