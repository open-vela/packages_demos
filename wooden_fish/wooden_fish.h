#ifndef __RRLATION_CAL_H__
#define __RRLATION_CAL_H__
#include <nuttx/config.h>
#include <lvgl/lvgl.h>
#include "audio_ctl.h"

#ifndef CONFIG_LVX_WOODEN_FISH_DATA_ROOT
#define CONFIG_LVX_WOODEN_FISH_DATA_ROOT "/data"
#endif

// 资源路径定义
#define WOODEN_FISH_RES_ROOT CONFIG_LVX_WOODEN_FISH_DATA_ROOT "/res"
#define WOODEN_FISH_IMG_PATH WOODEN_FISH_RES_ROOT "/wooden_fish.png"
#define WOODEN_FISH_ENTER_IMG_PATH WOODEN_FISH_RES_ROOT "/enter_white.png"
#define WOODEN_FISH_SOUND_PATH WOODEN_FISH_RES_ROOT "/knock.wav"
#define FONTS_ROOT WOODEN_FISH_RES_ROOT "/fonts"

/**
 * @struct wooden_fish_t
 * @brief 电子木鱼应用数据结构
 */
typedef struct wooden_fish_s {
    struct {
        lv_obj_t *screen;      // 主屏幕
        lv_obj_t *fish_img;    // 木鱼图片
        lv_obj_t *counter;     // 功德计数器
        lv_obj_t *auto_switch; // 自动敲击开关
        lv_obj_t *today_label; // 今日功德标签
    } ui;
    
    uint32_t knock_count;      // 敲击次数
    lv_timer_t *auto_timer;    // 自动敲击定时器
    const lv_font_t *font;           // 添加字体指针
} wooden_fish_t;

/**
 * @brief 创建电子木鱼应用
 */
void wooden_fish_app_create(void);
// 音频完成回调供 audio_ctl 调用
void wooden_fish_audio_complete_cb(void);
// 程序清理函数
void wooden_fish_cleanup(void);

#endif