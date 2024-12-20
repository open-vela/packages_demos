// include NuttX headers
#include <nuttx/config.h>
#include <unistd.h>
#include <uv.h>

// include lvgl headers
#include <lvgl/lvgl.h>

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

#include "music_player.h"

int main(int argc, FAR char* argv[])
{
    // init lvgl
    lv_nuttx_dsc_t info;
    lv_nuttx_result_t result;
    uv_loop_t ui_loop;
    lv_memset(&ui_loop, 0, sizeof(uv_loop_t));

    if (lv_is_initialized()) {
        LV_LOG_ERROR("LVGL already initialized! aborting.");
        return -1;
    }

    lv_init();

    lv_nuttx_dsc_init(&info);
    lv_nuttx_init(&info, &result);

    if (result.disp == NULL) {
        LV_LOG_ERROR("lv_demos initialization failure!");
        return 1;
    }

    app_create();

    // refresh lvgl ui
    lv_nuttx_uv_loop(&ui_loop, &result);

    lv_nuttx_deinit(&result);
    lv_deinit();

    return 0;
}
