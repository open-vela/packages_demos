# Vela Audio 技术文档

## 目录

- [技术架构](#技术架构)
- [API 参考](#api-参考)
- [开发指南](#开发指南)

## 技术架构

### 系统概览

Vela Audio 采用模块化分层架构设计，确保代码的可维护性、可扩展性和高性能。整个系统基于事件驱动模型，通过清晰的接口分离各个功能模块。

**核心设计原则**：
- **模块化**：每个功能模块独立设计，职责明确
- **分层架构**：清晰的层次结构，降低耦合度
- **事件驱动**：基于 LVGL 9.x 事件系统的响应式设计
- **资源管理**：统一的资源生命周期管理
- **状态机**：可预测的状态转换和管理

### 架构层次

系统采用六层架构设计，从下到上包括：

```
┌─────────────────────────────────────────┐
│           用户界面层 (LVGL 9.x)          │  ← 图形界面渲染和触屏交互
├─────────────────────────────────────────┤
│             UI模块层                    │  ← 启动页面、主界面、播放列表管理器
├─────────────────────────────────────────┤
│            业务逻辑层                    │  ← 状态管理、播放控制、播放列表逻辑
├─────────────────────────────────────────┤
│             服务层                      │  ← 音频控制、文件管理、Wi-Fi管理
├─────────────────────────────────────────┤
│        系统抽象层 (NuttX)                │  ← 音频驱动、文件系统、网络栈
├─────────────────────────────────────────┤
│           硬件抽象层                     │  ← 音频硬件、存储设备、网络硬件
└─────────────────────────────────────────┘
```

### 数据流图

系统的数据流主要包括 3 个主要路径：

#### 1. 应用启动流
```
main() → splash_screen_create() → 启动动画 → app_create() → 主界面
   ↓
资源初始化 → 字体加载 → 配置读取 → UI组件创建
```

#### 2. 用户交互流
```
用户操作 → LVGL 9.x 事件 → 事件处理函数 → 状态更新 → UI刷新
    ↓
音频控制 → 播放列表更新 → 界面反馈
```

#### 3. 模块通信流
```
主界面 ↔ 播放列表管理器 ↔ 音频控制器 ↔ 状态管理
   ↓              ↓              ↓
UI更新        列表显示        播放控制
```

### 主要数据结构

#### 运行时上下文
```c
struct ctx_s {
    bool resource_healthy_check;          // 资源健康检查标志
    album_info_t* current_album;          // 当前播放的专辑信息
    
    // 播放状态
    uint16_t volume;                      // 当前音量 (0-100)
    play_status_t play_status_prev;       // 上一次播放状态
    play_status_t play_status;            // 当前播放状态
    uint64_t current_time;                // 当前播放时间 (毫秒)
    
    // 定时器管理
    struct {
        lv_timer_t* volume_bar_countdown;      // 音量条自动隐藏倒计时
        lv_timer_t* playback_progress_update;  // 播放进度更新定时器
        lv_timer_t* refresh_date_time;         // 时间日期刷新定时器
    } timers;
    
    audioctl_s* audioctl;                 // 音频控制句柄
};
```

#### 资源管理结构
```c
struct resource_s {
    // UI组件管理
    struct {
        lv_obj_t* time;                    // 时间显示标签
        lv_obj_t* date;                    // 日期显示标签
        lv_obj_t* player_group;            // 播放器控件组
        lv_obj_t* album_cover_container;   // 圆形封面容器
        lv_obj_t* volume_bar;              // 音量条组件
        // ... 更多UI组件
    } ui;
    
    // 字体资源管理
    struct {
        struct { const lv_font_t* normal; } size_16;
        struct { const lv_font_t* bold; } size_22;
        struct { const lv_font_t* normal; } size_24;
        // ... 更多字体
    } fonts;
    
    // 音乐数据
    album_info_t* albums;                  // 专辑信息数组
    uint8_t album_count;                   // 专辑总数
};
```

### 状态机设计

#### 播放状态定义
```c
typedef enum {
    PLAY_STATUS_STOP,    // 停止状态
    PLAY_STATUS_PLAY,    // 播放状态
    PLAY_STATUS_PAUSE,   // 暂停状态
} play_status_t;
```

#### 状态转换表
```c
typedef struct {
    play_status_t from_state;     // 源状态
    play_status_t to_state;       // 目标状态
    void (*action)(void);         // 转换动作
} state_transition_t;

// 状态转换表
static const state_transition_t transitions[] = {
    {PLAY_STATUS_STOP,  PLAY_STATUS_PLAY,  action_start_playback},
    {PLAY_STATUS_PLAY,  PLAY_STATUS_PAUSE, action_pause_playback},
    {PLAY_STATUS_PAUSE, PLAY_STATUS_PLAY,  action_resume_playback},
    {PLAY_STATUS_PLAY,  PLAY_STATUS_STOP,  action_stop_playback},
    {PLAY_STATUS_PAUSE, PLAY_STATUS_STOP,  action_stop_playback},
};
```

## API 参考

### 核心 API

#### 应用程序控制
```c
/**
 * @brief 创建并初始化音乐播放器应用
 */
void app_create(void);

/**
 * @brief 设置播放状态
 * @param status 播放状态
 */
void app_set_play_status(play_status_t status);

/**
 * @brief 切换到指定专辑
 * @param index 专辑索引 (0-based)
 */
void app_switch_to_album(int index);
```

#### 启动页面模块 API
```c
/**
 * @brief 创建启动页面
 */
void splash_screen_create(void);
```

#### 播放列表管理器 API
```c
/**
 * @brief 创建专业播放列表界面
 * @param parent 父容器对象
 */
void playlist_manager_create(lv_obj_t* parent);

/**
 * @brief 刷新播放列表显示
 */
void playlist_manager_refresh(void);

/**
 * @brief 关闭播放列表
 */
void playlist_manager_close(void);

/**
 * @brief 检查播放列表是否打开
 * @return true 已打开, false 已关闭
 */
bool playlist_manager_is_open(void);
```

#### 音频控制 API
```c
/**
 * @brief 初始化音频控制器
 * @param file_path 音频文件路径
 * @return 音频控制句柄，失败返回NULL
 */
audioctl_s* audio_ctl_init_nxaudio(const char* file_path);

/**
 * @brief 开始播放
 * @param audioctl 音频控制句柄
 * @return 0 成功, -1 失败
 */
int audio_ctl_start(audioctl_s* audioctl);

/**
 * @brief 暂停播放
 * @param audioctl 音频控制句柄
 * @return 0 成功, -1 失败
 */
int audio_ctl_pause(audioctl_s* audioctl);

/**
 * @brief 停止播放
 * @param audioctl 音频控制句柄
 * @return 0 成功, -1 失败
 */
int audio_ctl_stop(audioctl_s* audioctl);

/**
 * @brief 设置音量
 * @param audioctl 音频控制句柄
 * @param volume 音量值 (0-100)
 * @return 0 成功, -1 失败
 */
int audio_ctl_set_volume(audioctl_s* audioctl, uint16_t volume);

/**
 * @brief 跳转到指定位置
 * @param audioctl 音频控制句柄
 * @param position 位置（秒）
 * @return 0 成功, -1 失败
 */
int audio_ctl_seek(audioctl_s* audioctl, uint32_t position);

/**
 * @brief 获取当前播放位置
 * @param audioctl 音频控制句柄
 * @return 当前位置（秒），-1表示失败
 */
int audio_ctl_get_position(audioctl_s* audioctl);
```

#### 字体配置 API
```c
/**
 * @brief 初始化字体系统
 */
void font_system_init(void);

/**
 * @brief 根据大小获取字体
 * @param size 字体大小
 * @return 字体指针，失败返回默认字体
 */
const lv_font_t* get_font_by_size(int size);

/**
 * @brief 设置UTF-8文本
 * @param label 标签对象
 * @param text 文本内容
 * @param font 字体
 */
void set_label_utf8_text(lv_obj_t* label, const char* text, const lv_font_t* font);
```

### 错误代码

```c
// 成功码
#define MUSIC_ERROR_OK              0    // 操作成功

// 通用错误码
#define MUSIC_ERROR_INVALID_PARAM  -1    // 无效参数
#define MUSIC_ERROR_NO_MEMORY      -2    // 内存不足
#define MUSIC_ERROR_TIMEOUT        -3    // 操作超时

// 文件相关错误码
#define MUSIC_ERROR_FILE_NOT_FOUND -10   // 文件未找到
#define MUSIC_ERROR_FILE_READ      -11   // 文件读取失败
#define MUSIC_ERROR_FILE_WRITE     -12   // 文件写入失败

// 音频相关错误码
#define MUSIC_ERROR_AUDIO_INIT     -20   // 音频初始化失败
#define MUSIC_ERROR_AUDIO_PLAY     -21   // 音频播放失败

// 网络相关错误码
#define MUSIC_ERROR_NETWORK        -30   // 网络错误
#define MUSIC_ERROR_WIFI_CONNECT   -31   // Wi-Fi连接失败

// UI相关错误码
#define MUSIC_ERROR_UI_INIT        -50   // UI初始化失败
#define MUSIC_ERROR_FONT_LOAD      -51   // 字体加载失败
```

## 开发指南

### 代码规范

#### C 代码风格
```c
// 函数命名：snake_case
static void app_create_main_page(void);
static bool init_audio_system(void);

// 变量命名：snake_case
static bool resource_healthy_check = false;
static uint32_t current_playback_time = 0;

// 常量命名：UPPER_CASE
#define MAX_ALBUM_COUNT 100
#define DEFAULT_VOLUME 50

// 结构体命名：snake_case_t
typedef struct album_info_s {
    const char* name;
    const char* artist;
    uint32_t duration;
} album_info_t;
```

#### 注释规范
```c
/**
 * @brief 函数简要描述
 * @param param1 参数1描述
 * @param param2 参数2描述
 * @return 返回值描述
 */
int function_name(int param1, const char* param2);

// 单行注释用于简单说明
int volume = 50; // Default volume level
```

#### 错误处理规范
```c
// 统一的错误处理模式
int audio_operation(audioctl_s* ctl) {
    // 参数验证
    if (!ctl) {
        return MUSIC_ERROR_INVALID_PARAM;
    }
    
    // 执行操作
    int result = low_level_operation(ctl);
    if (result < 0) {
        return MUSIC_ERROR_AUDIO_PLAY;
    }
    
    return MUSIC_ERROR_OK;
}
```

### 测试框架

#### 单元测试
```c
// test/test_audio_ctl.c
#include "unity.h"
#include "audio_ctl.h"

void test_audio_ctl_init_valid_file(void) {
    audioctl_s* ctl = audio_ctl_init_nxaudio("test.mp3");
    TEST_ASSERT_NOT_NULL(ctl);
    audio_ctl_uninit_nxaudio(ctl);
}

void test_audio_ctl_init_invalid_file(void) {
    audioctl_s* ctl = audio_ctl_init_nxaudio("nonexistent.mp3");
    TEST_ASSERT_NULL(ctl);
}
```

### 性能考量

#### 内存管理
```c
// 内存池管理
static uint8_t audio_buffer_pool[AUDIO_BUFFER_SIZE * 4];

static void* allocate_audio_buffer(size_t size) {
    // 简单的内存池分配逻辑
    return audio_buffer_pool;
}
```

#### UI性能优化
```c
// 减少重绘频率
static uint32_t last_ui_update = 0;

void update_ui_if_needed(void) {
    uint32_t now = lv_tick_get();
    if (now - last_ui_update < UI_UPDATE_INTERVAL_MS) {
        return; // 跳过本次更新
    }
    
    // 执行UI更新
    app_refresh_playback_status();
    last_ui_update = now;
}
```

---

*最后更新时间：2025-09-29*  