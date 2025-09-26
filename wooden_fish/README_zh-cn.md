# 电子木鱼 Demo 快速上手

\[ [English](./README.md)  | 简体中文 \]

本文档详细介绍如何在 openvela 系统上为 QEMU 模拟器构建、部署和运行电子木鱼（Wooden Fish）演示应用程序。您将学习如何配置项目、编译固件、运行应用，并对功能进行自定义修改。

## 一、运行效果

电子木鱼应用提供以下功能：
- 🐠 **手动敲击**：点击木鱼图片进行敲击，增加功德值
- 🔄 **自动模式**：开启自动敲击开关，每2秒自动敲击一次
- 💻 **程序猿模式**：切换到程序猿主题，功德变为PR（Pull Request）
- 🔊 **音效反馈**：每次敲击播放木鱼音效
- ✨ **动画效果**：敲击时显示"功德+1"或"pr+1"飞字动画

![电子木鱼演示](res/wooden_fish_demo.gif)

## 二、构建与运行

本节将指导您完成从项目配置到在目标平台上运行应用程序的完整流程。

### 准备工作

在开始之前，请确保您已位于 `openvela` 仓库的根目录下。本文中的所有命令均以此为起点。

### 步骤 1：配置项目

您需要通过 `menuconfig` 工具来启用电子木鱼应用并进行相关设置。

1. 启动 `menuconfig`。请根据您的目标平台选择对应的命令：

    - QEMU 模拟器:

        ```Bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
        ```

2. 在 `menuconfig` 界面中，按 `/` 键打开搜索功能，查找并启用以下配置项：

    ```Bash
    LVX_USE_DEMO_WOODEN_FISH=y
    LVX_WOODEN_FISH_DATA_ROOT="/data"
    ```

3. **（可选）** 如果您在运行时遇到界面卡顿或显示不流畅的问题，可以尝试增加 LVGL (Light and Versatile Graphics Library) 的缓存大小。搜索 `lv_cache_def_size` 并将其值设置为 `20000000`。

4. **（音频支持）** 确保启用音频相关配置以支持木鱼音效：
    ```Bash
    CONFIG_AUDIO=y
    CONFIG_AUDIOUTILS=y
    CONFIG_AUDIOUTILS_NXAUDIO=y
    ```

5. 保存配置并退出 `menuconfig`。

### 步骤 2：编译项目

编译前，建议先清理旧的构建产物以避免潜在的构建错误。

1. 清理构建产物 (distclean):

    - QEMU 模拟器:

        ```Bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j8
        ```

2. 执行构建:

    - QEMU 模拟器:

        ```Bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j8
        ```


### 步骤 3：部署与运行

编译成功后，您可以将固件部署到目标平台并启动应用。

#### 选项 A：在 QEMU 模拟器中运行

1. 在 `openvela` 根目录，执行以下命令启动模拟器：

    ```Bash
    ./emulator.sh vela
    ```

2. 等待 openvela 终端（`openvela-ap>`）出现后，输入以下命令启动电子木鱼：

    ```Bash
    wooden_fish
    ```

## 三、应用定制

您可以根据需求修改应用的核心功能，例如替换资源或调整行为参数。

### 核心文件结构

- `wooden_fish.c`: 包含应用的核心逻辑，包括UI创建、事件处理、动画效果等。所有功能开发和修改都应在此文件中进行。
- `wooden_fish.h`: 定义应用的数据结构、配置参数和函数声明。
- `wooden_fish_main.c`: 作为应用程序的入口，负责创建并启动运行应用逻辑的任务。通常情况下，您无需修改此文件。
- `audio_ctl.c` / `audio_ctl.h`: 音频控制模块，负责音效播放。

### 修改资源文件

应用使用的图片、音频和字体资源位于 `res/` 目录下。

#### 图片资源
- `res/wooden_fish.png`: 传统木鱼图片
- `res/enter_white.png`: 程序猿模式的Enter键图片

您可以替换这些图片文件，但请确保：
- 图片格式为PNG
- 分辨率适合目标显示设备
- 文件名保持一致

#### 音频资源
- `res/knock.wav`: 敲击音效文件

要替换音效：
1. 准备WAV格式的音频文件
2. 确保采样率和位深度兼容（推荐16位，44.1kHz）
3. 替换 `res/knock.wav` 文件

#### 字体资源
- `res/fonts/MiSans-Normal.ttf`: 应用使用的字体文件

### 调整应用参数

#### 自动敲击频率
在 `auto_knock_timer_cb` 函数中，自动敲击的间隔由定时器控制：

```
// 启动自动敲击定时器 (每2秒一次)
app->auto_timer = lv_timer_create(auto_knock_timer_cb, 2000, app);
```

要调整频率：
- **加快频率**: 减小时间值（例如 `1500` 毫秒）
- **放慢频率**: 增大时间值（例如 `3000` 毫秒）

#### 防抖时间
在 `play_wooden_fish_sound` 函数中，防抖机制防止过快点击：

```C
// 防抖：500ms内只播放一次
if (current_time - last_play_time < 500) {
    return;
}
```

调整防抖时间来改变响应敏感度。

#### 动画效果
在 `create_merit_animation` 函数中可以调整飞字动画：

```C
// 调整动画时间和距离
lv_anim_set_values(&move_anim, label_y, label_y - 120);  // 移动距离
lv_anim_set_time(&move_anim, 1500);  // 动画时长
```

## 四、技术特性

### 音频系统
- 使用 NuttX 的 nxaudio 框架
- 支持WAV格式音频播放
- 实现了安全的资源管理，避免内存泄漏

### UI系统
- 基于 LVGL 图形库
- 响应式布局设计
- 流畅的动画效果

### 状态管理
- 单例模式管理音频资源
- 防抖机制提升用户体验
- 统一的资源清理机制

## 五、故障排除

### 常见问题

#### 音频无法播放
1. 检查音频配置是否正确启用
2. 确认 `res/knock.wav` 文件存在
3. 检查音频设备是否正常工作

#### 界面显示异常
1. 检查字体文件是否正确加载
2. 增加 LVGL 缓存大小
3. 确认图片资源路径正确

#### 应用崩溃
1. 检查内存配置是否足够
2. 查看串口输出的错误信息
3. 确认所有依赖项已正确编译

### 调试方法

启用调试日志：
```
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_TRACE
```

查看运行时日志以获取详细的调试信息。

## 六、贡献指南

欢迎提交改进建议和功能增强！在提交PR前，请确保：

1. 代码风格符合项目规范
2. 添加了适当的注释和文档
3. 测试了基本功能的正常运行
4. 更新了相关的README文档

---

**注意**：本文档基于 openvela 开发环境编写。如遇到平台相关问题，请参考 openvela 官方文档或在项目仓库中提交Issue。
