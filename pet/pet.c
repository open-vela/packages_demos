//
// Virtual Pet Demo Application
//

/*********************
 *      INCLUDES
 *********************/
#include "pet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/* Init functions */
static bool init_resource(void);
static void app_create_error_page(void);
static void app_create_main_page(void);
static void init_pet_data(void);

/* UI Creation functions */
static void create_status_bar(void);
static void create_pet_area(void);
static void create_unified_control_panel(void);

/* Timer starting functions */
static void app_start_updating_timers(void);

/* Pet status and operations */
static void update_pet_mood(void);
static void update_pet_status(lv_timer_t *timer);
static void reset_pet_state(lv_timer_t *timer);
static void update_time_display(lv_timer_t *timer);
static void feed_pet(lv_event_t *e);
static void give_water(lv_event_t *e);
static void exercise_pet(lv_event_t *e);
static void toggle_sleep(lv_event_t *e);
static void close_msgbox_timer_cb(lv_timer_t* timer);

/* Data persistence functions */
static void save_pet_data(void);
static bool load_pet_data(void);

/* Achievement functions */
static void add_achievement(uint32_t achievement);
static void check_achievements(void);
static void init_random_event(void);

/* Utility functions */
static uint32_t calc_experience_for_level(uint8_t level);
static void update_visuals(void);

/**********************
 *  STATIC VARIABLES
 **********************/

// clang-format off
static pet_res_t   R;  /**< Resources */
static pet_ctx_t   C;  /**< Context */
static pet_conf_t  CF; /**< Configuration */
// clang-format on

/* Transition properties for the objects */
static const lv_style_prop_t transition_props[] = {
    LV_STYLE_OPA,
    LV_STYLE_BG_OPA,
    LV_STYLE_Y,
    LV_STYLE_HEIGHT,
    LV_STYLE_PROP_FLAG_NONE
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * 应用创建入口函数
 */
void pet_app_create(void)
{
    // 初始化资源和上下文结构
    lv_memzero(&R, sizeof(R));
    lv_memzero(&C, sizeof(C));
    lv_memzero(&CF, sizeof(CF));
    
    // 初始化随机数生成器
    srand(time(NULL));
    
    // 初始化资源
    C.resource_healthy_check = init_resource();
    
    if (!C.resource_healthy_check) {
        app_create_error_page();
        return;
    }
    
    // 初始化宠物数据
    init_pet_data();
    
    // 创建UI
    app_create_main_page();
    
    // 更新一次界面
    update_visuals();
    
    // 启动定时器
    app_start_updating_timers();
    
    // 初始化随机事件
    init_random_event();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * 初始化资源
 */
static bool init_resource(void)
{
    // Fonts
    R.fonts.size_16.normal = lv_freetype_font_create(FONTS_ROOT "/MiSans-Normal.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 16, LV_FREETYPE_FONT_STYLE_NORMAL);
    R.fonts.size_22.bold = lv_freetype_font_create(FONTS_ROOT "/MiSans-Semibold.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 22, LV_FREETYPE_FONT_STYLE_NORMAL);
    R.fonts.size_24.normal = lv_freetype_font_create(FONTS_ROOT "/MiSans-Normal.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 24, LV_FREETYPE_FONT_STYLE_NORMAL);
    R.fonts.size_28.normal = lv_freetype_font_create(FONTS_ROOT "/MiSans-Semibold.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 38, LV_FREETYPE_FONT_STYLE_NORMAL);
    R.fonts.size_60.bold = lv_freetype_font_create(FONTS_ROOT "/MiSans-Semibold.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 60, LV_FREETYPE_FONT_STYLE_NORMAL);

    if (R.fonts.size_16.normal == NULL ||
        R.fonts.size_22.bold == NULL   ||
        R.fonts.size_24.normal == NULL ||
        R.fonts.size_28.normal == NULL ||
        R.fonts.size_60.bold == NULL ) {
        return false;
    }

    // Styles
    lv_style_init(&R.styles.button_default);
    lv_style_init(&R.styles.button_pressed);
    lv_style_set_opa(&R.styles.button_default, LV_OPA_COVER);
    lv_style_set_opa(&R.styles.button_pressed, LV_OPA_70);

    // transition animation
    lv_style_transition_dsc_init(&R.styles.transition_dsc, transition_props, &lv_anim_path_ease_in_out, 300, 0, NULL);
    lv_style_transition_dsc_init(&R.styles.button_transition_dsc, transition_props, &lv_anim_path_ease_in_out, 80, 0, NULL);
    lv_style_set_transition(&R.styles.button_default, &R.styles.button_transition_dsc);
    lv_style_set_transition(&R.styles.button_pressed, &R.styles.button_transition_dsc);

    // Images
    R.images.background = IMAGES_ROOT "/background.png";
    R.images.sad = IMAGES_ROOT "/sad.png";
    R.images.calm = IMAGES_ROOT "/calm.png";
    R.images.happy = IMAGES_ROOT "/happy.png";
    R.images.eat_img = IMAGES_ROOT "/eat.png";     // 宠物进食图片
    R.images.drink_img = IMAGES_ROOT "/drink.png"; // 宠物喝水图片
    R.images.sleep_img = IMAGES_ROOT "/sleep.png";

    R.images.eat = ICONS_ROOT "/eat.png";          // 喂食按钮图标
    R.images.drink = ICONS_ROOT "/drink.png";      // 喝水按钮图标
    R.images.exercise = ICONS_ROOT "/exercise.png";
    R.images.sleep_icon = ICONS_ROOT "/sleep.png";
    

    return true;
}

/**
 * 计算经验值
 */
static uint32_t calc_experience_for_level(uint8_t level)
{
    return level * level * 100;
}

/**
 * 创建错误页面
 */
static void app_create_error_page(void)
{
    lv_obj_t* root = lv_screen_active();
    lv_obj_t* label = lv_label_create(root);
    lv_obj_set_width(label, LV_PCT(80));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label, "Resource loading failed. \nPlease check the device and \nread the document for more details.");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_center(label);
}

/**
 * 创建主页面
 */
static void app_create_main_page(void)
{
    // 创建状态栏
    create_status_bar();
    
    // 创建宠物区域
    create_pet_area();
    
    // 创建新的合并UI面板
    create_unified_control_panel();
}

/**
 * 创建状态栏
 */
static void create_status_bar(void)
{
    lv_obj_t* root = lv_screen_active();
    
    // 设置根对象样式
    lv_obj_set_style_bg_color(root, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(root, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(root, 20, LV_PART_MAIN);
    
    // 创建状态栏
    lv_obj_t* status_bar = lv_obj_create(root);
    lv_obj_set_size(status_bar, LV_PCT(100), 40);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(status_bar, 0, 0);
    R.ui.status_bar = status_bar;
    
    // 时间标签
    lv_obj_t* time_label = lv_label_create(status_bar);
    lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 10, 0);
    lv_label_set_text(time_label, "00:00");
    lv_obj_set_style_text_font(time_label, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    R.ui.time_label = time_label;
    
    // 电量标签
    lv_obj_t* battery_label = lv_label_create(status_bar);
    lv_obj_align(battery_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(battery_label, "100%");
    lv_obj_set_style_text_font(battery_label, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(battery_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    R.ui.battery_label = battery_label;
    
    // 等级标签
    lv_obj_t* level_label = lv_label_create(status_bar);
    lv_obj_align(level_label, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_label_set_text_fmt(level_label, "Lv.%d", C.data.level);
    lv_obj_set_style_text_font(level_label, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(level_label, lv_color_hex(0xFFFF00), LV_PART_MAIN);
    R.ui.level_label = level_label;
}

/**
 * 创建宠物显示区域
 */
static void create_pet_area(void)
{
    lv_obj_t* root = lv_screen_active();
    
    // 创建宠物显示区域
    lv_obj_t* pet_area = lv_obj_create(root);
    lv_obj_set_size(pet_area, LV_PCT(100), LV_PCT(60));
    lv_obj_align_to(pet_area, R.ui.status_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(pet_area, LV_OPA_0, 0);
    lv_obj_set_style_border_width(pet_area, 0, 0);
    R.ui.pet_area = pet_area;
    
    // 获取区域高度，用于计算缩放比例
    int area_height = lv_obj_get_height(pet_area);
    int area_width = lv_obj_get_width(pet_area);
    int scale = LV_MIN(area_width, area_height) * 100 / 1024; // 计算适合的缩放比例
    scale = scale * 2; // 将缩放比例增大一倍
    
    // 背景图像
    lv_obj_t* bg_img = lv_img_create(pet_area);
    lv_img_set_src(bg_img, R.images.background);
    lv_obj_align(bg_img, LV_ALIGN_CENTER, 0, 0);
    lv_img_set_zoom(bg_img, scale); // 使用增大后的缩放比例
    R.ui.background_img = bg_img;
    
    // 宠物图像
    lv_obj_t* pet_img = lv_img_create(pet_area);
    lv_img_set_src(pet_img, R.images.calm); // 默认为平静状态
    lv_obj_align(pet_img, LV_ALIGN_CENTER, 0, 0);
    lv_img_set_zoom(pet_img, scale); // 使用增大后的缩放比例
    R.ui.pet_img = pet_img;
}

/**
 * 创建合并的控制面板(状态 + 操作按钮)
 */
static void create_unified_control_panel(void)
{
    lv_obj_t* root = lv_screen_active();
    
    // 创建控制面板
    lv_obj_t* control_panel = lv_obj_create(root);
    lv_obj_set_size(control_panel, LV_PCT(100), 180);
    lv_obj_align_to(control_panel, R.ui.pet_area, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(control_panel, lv_color_hex(0x222222), 0);
    lv_obj_set_style_pad_all(control_panel, 10, 0);
    R.ui.status_panel = control_panel; // 重用原状态面板引用
    
    // 创建左右布局
    lv_obj_set_flex_flow(control_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(control_panel, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // 左侧状态面板
    lv_obj_t* status_area = lv_obj_create(control_panel);
    lv_obj_set_size(status_area, LV_PCT(45), LV_PCT(100));
    lv_obj_set_style_bg_opa(status_area, LV_OPA_0, 0);
    lv_obj_set_style_border_width(status_area, 0, 0);
    lv_obj_set_style_pad_all(status_area, 5, 0);
    
    // 右侧按钮面板
    lv_obj_t* buttons_area = lv_obj_create(control_panel);
    lv_obj_set_size(buttons_area, LV_PCT(50), LV_PCT(100));
    lv_obj_set_style_bg_opa(buttons_area, LV_OPA_0, 0);
    lv_obj_set_style_border_width(buttons_area, 0, 0);
    lv_obj_set_style_pad_all(buttons_area, 5, 0);
    
    // 在左侧添加状态条
    // 饥饿度条
    lv_obj_t* hunger_label = lv_label_create(status_area);
    lv_label_set_text(hunger_label, "饥饿:");
    lv_obj_set_pos(hunger_label, 0, 10);
    lv_obj_set_style_text_font(hunger_label, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(hunger_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_t* hunger_bar = lv_bar_create(status_area);
    lv_obj_set_size(hunger_bar, 180, 15);
    lv_obj_set_pos(hunger_bar, 60, 10);
    lv_bar_set_range(hunger_bar, 0, 100);
    lv_bar_set_value(hunger_bar, C.data.hunger, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(hunger_bar, lv_color_hex(0xFFAA00), LV_PART_INDICATOR);
    R.ui.hunger_bar = hunger_bar;
    
    // 口渴度条
    lv_obj_t* thirst_label = lv_label_create(status_area);
    lv_label_set_text(thirst_label, "口渴:");
    lv_obj_set_pos(thirst_label, 0, 40);
    lv_obj_set_style_text_font(thirst_label, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(thirst_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_t* thirst_bar = lv_bar_create(status_area);
    lv_obj_set_size(thirst_bar, 180, 15);
    lv_obj_set_pos(thirst_bar, 60, 40);
    lv_bar_set_range(thirst_bar, 0, 100);
    lv_bar_set_value(thirst_bar, C.data.thirst, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(thirst_bar, lv_color_hex(0x00AAFF), LV_PART_INDICATOR);
    R.ui.thirst_bar = thirst_bar;
    
    // 心情度条
    lv_obj_t* happiness_label = lv_label_create(status_area);
    lv_label_set_text(happiness_label, "心情:");
    lv_obj_set_pos(happiness_label, 0, 70);
    lv_obj_set_style_text_font(happiness_label, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(happiness_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_t* happiness_bar = lv_bar_create(status_area);
    lv_obj_set_size(happiness_bar, 180, 15);
    lv_obj_set_pos(happiness_bar, 60, 70);
    lv_bar_set_range(happiness_bar, 0, 100);
    lv_bar_set_value(happiness_bar, C.data.happiness, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(happiness_bar, lv_color_hex(0xFF5555), LV_PART_INDICATOR);
    R.ui.happiness_bar = happiness_bar;
    
    // 经验条
    lv_obj_t* exp_label = lv_label_create(status_area);
    lv_label_set_text(exp_label, "经验:");
    lv_obj_set_pos(exp_label, 0, 100);
    lv_obj_set_style_text_font(exp_label, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(exp_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_t* exp_bar = lv_bar_create(status_area);
    lv_obj_set_size(exp_bar, 180, 15);
    lv_obj_set_pos(exp_bar, 60, 100);
    lv_bar_set_range(exp_bar, 0, C.data.next_level_exp);
    lv_bar_set_value(exp_bar, C.data.experience, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(exp_bar, lv_color_hex(0x55FF55), LV_PART_INDICATOR);
    R.ui.exp_bar = exp_bar;
    
    // 为按钮区域设置水平行布局
    lv_obj_set_flex_flow(buttons_area, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons_area, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // 喂食按钮 - 直接使用图像而不是按钮背景
    lv_obj_t* feed_btn = lv_img_create(buttons_area);
    lv_img_set_src(feed_btn, R.images.eat);
    lv_obj_set_size(feed_btn, 88, 88);
    lv_obj_add_event_cb(feed_btn, feed_pet, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(feed_btn, LV_OBJ_FLAG_CLICKABLE); // 设置图像可点击
    lv_obj_set_style_img_recolor(feed_btn, lv_color_hex(0xFFFFFF), LV_STATE_PRESSED); // 点击时的反馈
    lv_obj_set_style_img_recolor_opa(feed_btn, LV_OPA_30, LV_STATE_PRESSED);
    R.ui.feed_btn = feed_btn;
    
    // 喝水按钮
    lv_obj_t* water_btn = lv_img_create(buttons_area);
    lv_img_set_src(water_btn, R.images.drink);
    lv_obj_set_size(water_btn, 88, 88);
    lv_obj_add_event_cb(water_btn, give_water, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(water_btn, LV_OBJ_FLAG_CLICKABLE); // 设置图像可点击
    lv_obj_set_style_img_recolor(water_btn, lv_color_hex(0xFFFFFF), LV_STATE_PRESSED); // 点击时的反馈
    lv_obj_set_style_img_recolor_opa(water_btn, LV_OPA_30, LV_STATE_PRESSED);
    R.ui.water_btn = water_btn;
    
    // 运动按钮
    lv_obj_t* exercise_btn = lv_img_create(buttons_area);
    lv_img_set_src(exercise_btn, R.images.exercise);
    lv_obj_set_size(exercise_btn, 88, 88);
    lv_obj_add_event_cb(exercise_btn, exercise_pet, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(exercise_btn, LV_OBJ_FLAG_CLICKABLE); // 设置图像可点击
    lv_obj_set_style_img_recolor(exercise_btn, lv_color_hex(0xFFFFFF), LV_STATE_PRESSED); // 点击时的反馈
    lv_obj_set_style_img_recolor_opa(exercise_btn, LV_OPA_30, LV_STATE_PRESSED);
    R.ui.exercise_btn = exercise_btn;
    
    // 睡眠按钮
    lv_obj_t* sleep_btn = lv_img_create(buttons_area);
    lv_img_set_src(sleep_btn, R.images.sleep_icon);
    lv_obj_set_size(sleep_btn, 88, 88);
    lv_obj_add_event_cb(sleep_btn, toggle_sleep, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(sleep_btn, LV_OBJ_FLAG_CLICKABLE); // 设置图像可点击 
    lv_obj_set_style_img_recolor(sleep_btn, lv_color_hex(0xFFFFFF), LV_STATE_PRESSED); // 点击时的反馈
    lv_obj_set_style_img_recolor_opa(sleep_btn, LV_OPA_30, LV_STATE_PRESSED);
    R.ui.sleep_btn = sleep_btn;
}

// 初始化宠物数据
static void init_pet_data(void)
{
    // 尝试加载保存的数据，如果失败则初始化默认值
    if (!load_pet_data()) {
        C.data.level = 1;
        C.data.hunger = 60;    // 初始饥饿度为60%
        C.data.thirst = 60;    // 初始口渴度为60%
        C.data.happiness = 60; // 初始心情值为60%
        C.data.experience = 0; // 经验值从0开始
        C.data.next_level_exp = calc_experience_for_level(2);
        C.data.mood = PET_MOOD_CALM;
        C.data.state = PET_STATE_NORMAL;
        C.data.sleeping = false;
        C.data.days_alive = 0;
        C.data.achievements = 0;
    }
    
    C.data.last_update = time(NULL);
    C.start_time = time(NULL);
    C.initialized = true;
}

// 更新时间显示
void update_time_display(lv_timer_t *timer)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    char time_str[16];
    strftime(time_str, sizeof(time_str), "%H:%M", tm_info);
    
    lv_label_set_text(R.ui.time_label, time_str);
    
    // 模拟电池电量(真实设备应该读取系统电量)
    static int battery = 100;
    if (rand() % 100 > 95) {
        // 偶尔减少电量
        battery = (battery > 5) ? battery - 1 : 5;
    }
    lv_label_set_text_fmt(R.ui.battery_label, "%d%%", battery);
    
    // 每天更新一次天数
    static int last_day = -1;
    if (tm_info->tm_mday != last_day) {
        if (last_day != -1) {
            C.data.days_alive++;
            check_achievements();
        }
        last_day = tm_info->tm_mday;
    }
}

// 更新宠物心情状态
static void update_pet_mood()
{
    // 根据幸福度确定心情
    if (C.data.happiness < MOOD_SAD_THRESHOLD) {
        C.data.mood = PET_MOOD_SAD;
    } else if (C.data.happiness > MOOD_HAPPY_THRESHOLD) {
        C.data.mood = PET_MOOD_HAPPY;
    } else {
        C.data.mood = PET_MOOD_CALM;
    }
}

// 更新宠物状态
void update_pet_status(lv_timer_t *timer)
{
    if (!C.initialized) return;
    
    time_t current_time = time(NULL);
    double elapsed_minutes = difftime(current_time, C.data.last_update) / 60.0;
    
    if (elapsed_minutes < 0.01) return; // 避免频繁更新
    
    // 更新状态值
    if (!C.data.sleeping) {
        // 正常状态下的变化
        C.data.hunger = (C.data.hunger > HUNGER_DECREASE_RATE * elapsed_minutes) 
            ? C.data.hunger - HUNGER_DECREASE_RATE * elapsed_minutes : 0;
        
        C.data.thirst = (C.data.thirst > THIRST_DECREASE_RATE * elapsed_minutes)
            ? C.data.thirst - THIRST_DECREASE_RATE * elapsed_minutes : 0;
        
        // 心情变化逻辑
        double happiness_change = HAPPINESS_DECREASE_RATE * elapsed_minutes;
        
        // 如果饥饿或口渴低于阈值，心情下降更快
        if (C.data.hunger < HUNGER_THRESHOLD || C.data.thirst < THIRST_THRESHOLD) {
            happiness_change *= 2;
        }
        
        C.data.happiness = (C.data.happiness > happiness_change)
            ? C.data.happiness - happiness_change : 0;
    } else {
        // 睡眠状态下的变化(大幅减缓下降速度)
        C.data.hunger = (C.data.hunger > HUNGER_DECREASE_RATE * elapsed_minutes * 0.3) 
            ? C.data.hunger - HUNGER_DECREASE_RATE * elapsed_minutes * 0.3 : 0;
        
        C.data.thirst = (C.data.thirst > THIRST_DECREASE_RATE * elapsed_minutes * 0.3)
            ? C.data.thirst - THIRST_DECREASE_RATE * elapsed_minutes * 0.3 : 0;
        
        // 睡眠时心情缓慢恢复
        C.data.happiness = (C.data.happiness + HAPPINESS_DECREASE_RATE * elapsed_minutes * 1.5 < 100)
            ? C.data.happiness + HAPPINESS_DECREASE_RATE * elapsed_minutes * 1.5 : 100;
    }
    
    // 经验值增长
    C.data.experience += EXP_INCREASE_RATE * elapsed_minutes;
    
    // 检查是否升级
    if (C.data.experience >= C.data.next_level_exp) {
        C.data.level++;
        C.data.experience = 0; // 升级后经验值清零
        C.data.next_level_exp = calc_experience_for_level(C.data.level + 1);
        lv_label_set_text_fmt(R.ui.level_label, "Lv.%d", C.data.level);
        
        // 升级成就检查
        if (C.data.level == 5) {
            add_achievement(ACHIEVEMENT_REACH_LEVEL_5);
        } else if (C.data.level == 10) {
            add_achievement(ACHIEVEMENT_REACH_LEVEL_10);
        }
    }
    
    // 更新心情状态
    update_pet_mood();
    
    // 更新上次更新时间
    C.data.last_update = current_time;
    
    // 更新UI显示
    update_visuals();
    
    // 定期保存数据
    static int save_counter = 0;
    save_counter++;
    if (save_counter >= 60) { // 每60次更新保存一次(约1分钟)
        save_pet_data();
        save_counter = 0;
    }
}

// 更新视觉显示
void update_visuals(void)
{
    // 更新状态条
    lv_bar_set_value(R.ui.hunger_bar, C.data.hunger, LV_ANIM_ON);
    lv_bar_set_value(R.ui.thirst_bar, C.data.thirst, LV_ANIM_ON);
    lv_bar_set_value(R.ui.happiness_bar, C.data.happiness, LV_ANIM_ON);
    
    // 更新经验条
    lv_bar_set_range(R.ui.exp_bar, 0, C.data.next_level_exp);
    lv_bar_set_value(R.ui.exp_bar, C.data.experience, LV_ANIM_ON);
    
    // 根据状态更新宠物图像
    if (C.data.sleeping) {
        lv_img_set_src(R.ui.pet_img, R.images.sleep_img);
    } else {
        // 根据当前状态显示不同图像
        switch (C.data.state) {
            case PET_STATE_EATING:
                lv_img_set_src(R.ui.pet_img, R.images.eat_img);
                break;
            case PET_STATE_DRINKING:
                lv_img_set_src(R.ui.pet_img, R.images.drink_img);
                break;
            case PET_STATE_EXERCISING:
                // 运动时根据心情显示不同图像
                switch (C.data.mood) {
                    case PET_MOOD_SAD:
                        lv_img_set_src(R.ui.pet_img, R.images.sad);
                        break;
                    case PET_MOOD_HAPPY:
                        lv_img_set_src(R.ui.pet_img, R.images.happy);
                        break;
                    case PET_MOOD_CALM:
                    default:
                        lv_img_set_src(R.ui.pet_img, R.images.calm);
                        break;
                }
                break;
            case PET_STATE_NORMAL:
            default:
                // 正常状态下根据心情显示不同图像
                switch (C.data.mood) {
                    case PET_MOOD_SAD:
                        lv_img_set_src(R.ui.pet_img, R.images.sad);
                        break;
                    case PET_MOOD_HAPPY:
                        lv_img_set_src(R.ui.pet_img, R.images.happy);
                        break;
                    case PET_MOOD_CALM:
                    default:
                        lv_img_set_src(R.ui.pet_img, R.images.calm);
                        break;
                }
                break;
        }
    }
}

// 重置宠物状态(用于动作结束后)
void reset_pet_state(lv_timer_t *timer)
{
    // 将状态恢复到正常
    C.data.state = PET_STATE_NORMAL;
    
    // 更新视觉显示，恢复到基于心情的图像
    update_visuals();
    
    R.timers.state_timer = NULL;
}

// 喂食事件
void feed_pet(lv_event_t *e)
{
    if (C.data.sleeping || C.data.state != PET_STATE_NORMAL)
        return;
    
    C.data.state = PET_STATE_EATING;
    C.data.hunger = (C.data.hunger + 20 <= 100) ? C.data.hunger + 20 : 100;
    C.data.happiness = (C.data.happiness + 5 <= 100) ? C.data.happiness + 5 : 100;
    
    static int feed_days = 0;
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    static int last_feed_day = -1;
    
    if (tm_now->tm_mday != last_feed_day) {
        feed_days++;
        last_feed_day = tm_now->tm_mday;
        
        // 检查连续7天喂食成就
        if (feed_days >= 7) {
            add_achievement(ACHIEVEMENT_FEED_7_DAYS);
        }
    }
    
    // 更新显示状态数据，但不再更改宠物图像
    update_visuals();
    
    // 创建状态重置定时器
    if (R.timers.state_timer != NULL) {
        lv_timer_del(R.timers.state_timer);
    }
    R.timers.state_timer = lv_timer_create(reset_pet_state, STATE_DURATION, NULL);
}

// 喝水事件
void give_water(lv_event_t *e)
{
    if (C.data.sleeping || C.data.state != PET_STATE_NORMAL)
        return;
    
    C.data.state = PET_STATE_DRINKING;
    C.data.thirst = (C.data.thirst + 20 <= 100) ? C.data.thirst + 20 : 100;
    C.data.happiness = (C.data.happiness + 5 <= 100) ? C.data.happiness + 5 : 100;
    
    static int water_days = 0;
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    static int last_water_day = -1;
    
    if (tm_now->tm_mday != last_water_day) {
        water_days++;
        last_water_day = tm_now->tm_mday;
        
        // 检查连续7天喝水成就
        if (water_days >= 7) {
            add_achievement(ACHIEVEMENT_WATER_7_DAYS);
        }
    }
    
    // 更新显示状态数据，但不再更改宠物图像
    update_visuals();
    
    // 创建状态重置定时器
    if (R.timers.state_timer != NULL) {
        lv_timer_del(R.timers.state_timer);
    }
    R.timers.state_timer = lv_timer_create(reset_pet_state, STATE_DURATION, NULL);
}

// 运动事件
void exercise_pet(lv_event_t *e)
{
    if (C.data.sleeping || C.data.state != PET_STATE_NORMAL)
        return;
    
    // 检查饥饿度和口渴度，如果任一为0则不允许运动
    if (C.data.hunger <= 0 || C.data.thirst <= 0) {
        // 创建提示消息
        lv_obj_t* msg = lv_msgbox_create(NULL);
        if (C.data.hunger <= 0 && C.data.thirst <= 0) {
            lv_msgbox_add_title(msg, "Can't exercise");
            lv_msgbox_add_text(msg, "The pet is too hungry and thirsty, can't exercise!\nPlease feed and drink first.");
        } else if (C.data.hunger <= 0) {
            lv_msgbox_add_title(msg, "Can't exercise");
            lv_msgbox_add_text(msg, "The pet is too hungry, can't exercise!\nPlease feed first.");
        } else {
            lv_msgbox_add_title(msg, "Can't exercise");
            lv_msgbox_add_text(msg, "The pet is too thirsty, can't exercise!\nPlease drink first.");
        }
        lv_obj_center(msg);
        
        // 自动关闭消息框
        lv_timer_t* timer = lv_timer_create(close_msgbox_timer_cb, 2000, msg);
        lv_timer_set_repeat_count(timer, 1);
        return;
    }
    
    C.data.state = PET_STATE_EXERCISING;
    C.data.hunger = (C.data.hunger > 10) ? C.data.hunger - 10 : 0;
    C.data.thirst = (C.data.thirst > 15) ? C.data.thirst - 15 : 0;
    C.data.happiness = (C.data.happiness + 15 <= 100) ? C.data.happiness + 15 : 100;
    C.data.experience += 20; // 运动可以获得额外经验
    
    static int exercise_days = 0;
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    static int last_exercise_day = -1;
    
    if (tm_now->tm_mday != last_exercise_day) {
        exercise_days++;
        last_exercise_day = tm_now->tm_mday;
        
        // 检查连续7天运动成就
        if (exercise_days >= 7) {
            add_achievement(ACHIEVEMENT_EXERCISE_7_DAYS);
        }
    }
    
    // 更新显示状态数据，但不再更改宠物图像
    update_visuals();
    
    // 创建状态重置定时器
    if (R.timers.state_timer != NULL) {
        lv_timer_del(R.timers.state_timer);
    }
    R.timers.state_timer = lv_timer_create(reset_pet_state, STATE_DURATION, NULL);
}

// 睡眠切换事件
void toggle_sleep(lv_event_t *e)
{
    if (C.data.state != PET_STATE_NORMAL)
        return;
    
    C.data.sleeping = !C.data.sleeping;
    update_visuals();
}

// 定时器回调函数
static void close_msgbox_timer_cb(lv_timer_t* timer)
{
    lv_msgbox_close(timer->user_data);
}

// 添加成就
void add_achievement(uint32_t achievement)
{
    if ((C.data.achievements & achievement) == 0) {
        C.data.achievements |= achievement;
        
        // 显示成就提示
        lv_obj_t* achievement_msg = lv_msgbox_create(NULL);
        lv_msgbox_add_title(achievement_msg, "New achievement unlocked!");
        
        const char* achievement_text = NULL;
        switch (achievement) {
            case ACHIEVEMENT_FEED_7_DAYS:
                achievement_text = "Feed for 7 days!";
                break;
            case ACHIEVEMENT_WATER_7_DAYS:
                achievement_text = "Drink for 7 days!";
                break;
            case ACHIEVEMENT_EXERCISE_7_DAYS:
                achievement_text = "Exercise for 7 days!";
                break;
            case ACHIEVEMENT_SURVIVE_30_DAYS:
                achievement_text = "Pet for 30 days!";
                break;
            case ACHIEVEMENT_REACH_LEVEL_5:
                achievement_text = "Pet level 5!";
                break;
            case ACHIEVEMENT_REACH_LEVEL_10:
                achievement_text = "Pet level 10!";
                break;
            default:
                achievement_text = "New achievement unlocked!";
                break;
        }
        
        lv_msgbox_add_text(achievement_msg, achievement_text);
        lv_obj_center(achievement_msg);
        
        // 自动关闭消息框
        lv_timer_t* timer = lv_timer_create(close_msgbox_timer_cb, 3000, achievement_msg);
        lv_timer_set_repeat_count(timer, 1);
    }
}

// 检查成就
void check_achievements(void)
{
    // 检查存活天数成就
    if (C.data.days_alive >= 30) {
        add_achievement(ACHIEVEMENT_SURVIVE_30_DAYS);
    }
}

// 随机事件初始化
void init_random_event(void)
{
    // 随机事件将在未来的版本中实现
    // 例如：宠物生病、遇到玩伴等特殊事件
}

// 保存宠物数据到文件
void save_pet_data(void)
{
    char save_path[256];
    snprintf(save_path, sizeof(save_path), "%s/pet_data.bin", RES_ROOT);
    
    int fd = open(save_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        LV_LOG_ERROR("Failed to open save file for writing");
        return;
    }
    
    // 写入数据版本号
    uint32_t version = 1;
    write(fd, &version, sizeof(version));
    
    // 写入宠物数据
    write(fd, &C.data, sizeof(C.data));
    
    close(fd);
    LV_LOG_INFO("Pet data saved successfully");
}

// 从文件加载宠物数据
bool load_pet_data(void)
{
    char save_path[256];
    snprintf(save_path, sizeof(save_path), "%s/pet_data.bin", RES_ROOT);
    
    int fd = open(save_path, O_RDONLY);
    if (fd < 0) {
        LV_LOG_WARN("No save file found, starting with default values");
        return false;
    }
    
    // 读取数据版本号
    uint32_t version;
    if (read(fd, &version, sizeof(version)) != sizeof(version)) {
        LV_LOG_ERROR("Failed to read version from save file");
        close(fd);
        return false;
    }
    
    if (version != 1) {
        LV_LOG_ERROR("Incompatible save file version");
        close(fd);
        return false;
    }
    
    // 读取宠物数据
    if (read(fd, &C.data, sizeof(C.data)) != sizeof(C.data)) {
        LV_LOG_ERROR("Failed to read pet data from save file");
        close(fd);
        return false;
    }
    
    close(fd);
    LV_LOG_INFO("Pet data loaded successfully");
    C.data_loaded = true;
    return true;
}

/**
 * 启动更新定时器
 */
static void app_start_updating_timers(void)
{
    R.timers.update_timer = lv_timer_create(update_pet_status, TIMER_UPDATE_PERIOD, NULL);
    R.timers.time_update_timer = lv_timer_create(update_time_display, 1000, NULL);
}
