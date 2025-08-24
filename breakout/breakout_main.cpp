/****************************************************
 * breakout_main.cpp
 * Main program entry for the breakout game on NuttX OS.
 ***************************************************/

#include <nuttx/config.h>
#include <unistd.h>
#include <uv.h>
#include "src/breakout.h"
#include <lvgl/lvgl.h>

/**
 * A helper function to initialize and run the libuv event loop.
 * The loop handles input events, timers, and screen refreshes.
 */
static void lv_nuttx_uv_loop(uv_loop_t* loop, lv_nuttx_result_t* result)
{
    // Create a struct to pass integration info between NuttX and LVGL
    lv_nuttx_uv_t uv_info;
    void* data;

    // Initialize the libuv event loop
    uv_loop_init(loop);

    // Prepare config info to pass to the LVGL-UV integration layer
    lv_memset(&uv_info, 0, sizeof(uv_info));
    uv_info.loop = loop;
    uv_info.disp = result->disp;    // Pass in the initialized display device
    uv_info.indev = result->indev;  // Pass in the initialized input device (touchscreen)
#ifdef CONFIG_UINPUT_TOUCH
    uv_info.uindev = result->utouch_indev; // If configured, also pass virtual user input device
#endif
    data = lv_nuttx_uv_init(&uv_info);
    uv_run(loop, UV_RUN_DEFAULT);
    lv_nuttx_uv_deinit(&data);
}

/**************************************************
 * Application main entry function.
 ***************************************************/
extern "C" int breakout_main(int argc, FAR char *argv[])
{
    lv_nuttx_dsc_t info;
    lv_nuttx_result_t result;
    uv_loop_t ui_loop;
    lv_memset(&ui_loop, 0, sizeof(uv_loop_t));

    // Safety check: ensure LVGL is not initialized multiple times
    if (lv_is_initialized()) {
        LV_LOG_ERROR("LVGL already initialized! aborting.");
        return -1;
    }

    // Initialize LVGL core library
    lv_init();

    // Initialize and bind NuttX display and input drivers
    lv_nuttx_dsc_init(&info);
    lv_nuttx_init(&info, &result);

    // Check if the display device was successfully initialized
    if (result.disp == NULL) {
        LV_LOG_ERROR("lv_demos initialization failure!");
        return 1;
    }

    // Call the game start function to create the game UI and all objects
    ballgame_start();

    // Start the main event loop driving the entire UI and application
    lv_nuttx_uv_loop(&ui_loop, &result);

    // After the application exits the loop, deinitialize all resources
    lv_nuttx_deinit(&result);
    lv_deinit();

    return 0;
}
