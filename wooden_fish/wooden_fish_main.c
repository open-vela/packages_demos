// include NuttX headers
#include <nuttx/config.h>
#include <unistd.h>
#include <signal.h>

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
#include <uv.h>
#endif

// include lvgl headers
#include <lvgl/lvgl.h>

#include "wooden_fish.h"

// 全局退出标志
static volatile bool g_exit_flag = false;

// 信号处理函数
static void signal_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        g_exit_flag = true;
    }
}

#ifdef CONFIG_LV_USE_NUTTX_LIBUV

static void lv_nuttx_uv_loop(uv_loop_t *loop, lv_nuttx_result_t *result)
{
    lv_nuttx_uv_t uv_info;
    void *data;

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

#endif

int main(int argc, FAR char *argv[])
{
    // init lvgl
    lv_nuttx_dsc_t info;
    lv_nuttx_result_t result;

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
    uv_loop_t ui_loop;
    lv_memset(&ui_loop, 0, sizeof(uv_loop_t));
#endif

    if (lv_is_initialized())
    {
        LV_LOG_ERROR("LVGL already initialized! aborting.");
        return -1;
    }

    lv_init();

    // 设置信号处理
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    lv_nuttx_dsc_init(&info);
    lv_nuttx_init(&info, &result);

    if (result.disp == NULL)
    {
        LV_LOG_ERROR("lv_demos initialization failure!");
        return 1;
    }

    wooden_fish_app_create();

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
    // refresh lvgl ui
    lv_nuttx_uv_loop(&ui_loop, &result);
#endif

    while (!g_exit_flag)
    {
        uint32_t idle;
        idle = lv_timer_handler();

        /* Minimum sleep of 1ms */

        idle = idle ? idle : 1;
        usleep(idle * 1000);
    }

    // 程序退出前清理资源
    wooden_fish_cleanup();
    
    lv_nuttx_deinit(&result);
    lv_deinit();

    return 0;
}
