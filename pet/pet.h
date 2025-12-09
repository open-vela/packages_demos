#ifndef VIRTUAL_PET_H
#define VIRTUAL_PET_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/*********************
 *      DEFINES
 *********************/
#define RES_ROOT CONFIG_LVX_PET_DATA_ROOT "/res"
#define IMAGES_ROOT RES_ROOT "/image"
#define FONTS_ROOT RES_ROOT "/fonts"
#define ICONS_ROOT RES_ROOT "/icons"

// 常量定义
#define HUNGER_THRESHOLD 30
#define THIRST_THRESHOLD 30
#define MOOD_SAD_THRESHOLD 30
#define MOOD_HAPPY_THRESHOLD 70
#define TIMER_UPDATE_PERIOD 1000   // 更新周期(ms)
#define STATE_DURATION 3000        // 状态持续时间(ms)
#define HUNGER_DECREASE_RATE 0.3   // 每分钟饥饿值下降，进一步减慢
#define THIRST_DECREASE_RATE 0.4   // 每分钟口渴值下降，进一步减慢
#define HAPPINESS_DECREASE_RATE 0.2  // 每分钟心情值下降，进一步减慢
#define EXP_INCREASE_RATE 10        // 每分钟经验值增加
#define ACHIEVEMENT_FEED_7_DAYS (1 << 0)
#define ACHIEVEMENT_WATER_7_DAYS (1 << 1)
#define ACHIEVEMENT_EXERCISE_7_DAYS (1 << 2)
#define ACHIEVEMENT_SURVIVE_30_DAYS (1 << 3)
#define ACHIEVEMENT_REACH_LEVEL_5 (1 << 4)
#define ACHIEVEMENT_REACH_LEVEL_10 (1 << 5)

/**********************
 *      TYPEDEFS
 **********************/

// 宠物状态
typedef enum {
    PET_MOOD_SAD = 0,
    PET_MOOD_CALM = 1,
    PET_MOOD_HAPPY = 2
} pet_mood_t;

typedef enum {
    PET_STATE_NORMAL = 0,
    PET_STATE_EATING = 1,
    PET_STATE_DRINKING = 2,
    PET_STATE_EXERCISING = 3,
    PET_STATE_SLEEPING = 4
} pet_state_t;

// 宠物信息结构体
typedef struct {
    uint8_t level;           // 宠物等级
    uint8_t hunger;          // 饥饿值 (0-100)
    uint8_t thirst;          // 口渴值 (0-100)
    uint8_t happiness;       // 心情值 (0-100)
    uint32_t experience;     // 经验值
    uint32_t next_level_exp; // 下一级所需经验
    pet_mood_t mood;         // 当前心情
    pet_state_t state;       // 当前状态
    time_t last_update;      // 上次更新时间
    bool sleeping;           // 是否睡眠
    int days_alive;          // 存活天数
    uint32_t achievements;   // 成就(位图)
} pet_data_t;

// UI资源结构体
typedef struct {
    struct {
        lv_obj_t *status_bar;
        lv_obj_t *time_label;
        lv_obj_t *battery_label;
        lv_obj_t *level_label;
        
        lv_obj_t *pet_area;
        lv_obj_t *pet_img;
        lv_obj_t *background_img;
        
        lv_obj_t *status_panel;
        lv_obj_t *hunger_bar;
        lv_obj_t *thirst_bar;
        lv_obj_t *happiness_bar;
        lv_obj_t *exp_bar;
        
        lv_obj_t *action_panel;
        lv_obj_t *feed_btn;
        lv_obj_t *water_btn;
        lv_obj_t *exercise_btn;
        lv_obj_t *sleep_btn;
    } ui;
    
    struct {
        struct {
            lv_font_t* normal;
        } size_16;
        struct {
            lv_font_t* bold;
        } size_22;
        struct {
            lv_font_t* normal;
        } size_24;
        struct {
            lv_font_t* normal;
        } size_28;
        struct {
            lv_font_t* bold;
        } size_60;
    } fonts;

    struct {
        lv_style_t button_default;
        lv_style_t button_pressed;
        lv_style_transition_dsc_t button_transition_dsc;
        lv_style_transition_dsc_t transition_dsc;
    } styles;
    
    struct {
        const char* background;
        const char* sad;
        const char* calm;
        const char* happy;
        const char* eat_img;     // 宠物进食图片
        const char* drink_img;   // 宠物喝水图片
        const char* eat;         // 喂食按钮图标
        const char* drink;       // 喝水按钮图标
        const char* exercise;
        const char* sleep_img;
        const char* sleep_icon;
    } images;
    
    struct {
        lv_timer_t *update_timer;
        lv_timer_t *state_timer;
        lv_timer_t *time_update_timer;
    } timers;
} pet_res_t;

// 应用上下文
typedef struct {
    pet_data_t data;
    bool initialized;
    time_t start_time;
    time_t action_start_time;
    bool data_loaded;
    bool resource_healthy_check;
    lv_obj_t *current_pet_related_obj;
} pet_ctx_t;

// 配置结构体
typedef struct {
    // 可扩展配置项
    int dummy;
} pet_conf_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void pet_app_create(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* VIRTUAL_PET_H */
