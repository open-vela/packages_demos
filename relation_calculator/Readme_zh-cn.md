# 亲戚计算器 Demo 快速上手

\[ [English](Readme.md) | 简体中文 \]

本文档为在 `openvela` 系统上构建、部署和运行**亲戚计算器**演示应用提供了全面的指导。内容涵盖 QEMU 模拟器和 ESP32-S3-BOX 开发板的操作流程，以及如何通过增加新关系来自定义此应用。

## 一、运行效果

![img](./img/show.gif)

## 二、构建与运行

本章节将引导您完成从项目配置到在目标平台上启动应用的完整过程。

### 准备工作

在开始之前，请确保您位于 `openvela` 仓库的根目录下。本指南中的所有命令均默认从该位置执行。

### 第一步：配置项目

使用 `menuconfig` 工具来启用**亲戚计算器**演示应用。

1. 根据您的目标平台，选择对应的命令启动 `menuconfig`：

    - **QEMU 模拟器:**

        ```Bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
        ```

    - **ESP32-S3-BOX 开发板:**

        ```Bash
        ./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 menuconfig
        ```

2. 在 `menuconfig` 界面中，按下 `/` 键打开搜索工具，查找并启用以下配置项：

    ```Bash
    LVX_USE_DEMO_RELATIVES_CALCULATOR=y
    ```

3. 保存您的配置并退出 `menuconfig`。

### 第二步：构建项目

为避免潜在的冲突，建议在开始新的编译前清理旧的构建产物。

1. 清理构建产物 (distclean):

    - **QEMU 模拟器:**

        ```Bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j8
        ```

    - **ESP32-S3-BOX 开发板:**

        ```Bash
        ./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 distclean -j8
        ```

2. 执行构建:

    - **QEMU 模拟器:**

        ```Bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j8
        ```

    - **ESP32-S3-BOX 开发板:**

        ```Bash
        ./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 -j8
        ```

### 第三步：部署并运行应用

成功构建后，请根据您的目标平台执行相应指令来部署固件并运行计算器。

#### 选项 A：在 QEMU 模拟器上运行

1. 在 `openvela` 根目录，执行以下命令启动模拟器：

    ```Bash
    ./emulator.sh vela
    ```

2. 等待 openvela 终端（`openvela-ap>`）出现后，运行以下命令在后台启动应用：

    ```Bash
    rel_cal &
    ```

#### 选项 B：在 ESP32-S3-BOX 开发板上运行

1. 烧录固件： 通过 USB 将 ESP32-S3-BOX 开发板连接到您的计算机。运行以下命令烧录固件，需将 `/dev/ttyACM0` 替换为您设备的实际串口号。

    ```Bash
    pushd nuttx && make -j8 flash ESPTOOL_PORT=/dev/ttyACM0 ESPTOOL_BINDIR=./ && popd
    ```

2. 打开串口终端： 使用 `minicom` 等串口终端工具来监控设备输出并与 Shell 交互。

    ```Bash
    sudo minicom -D /dev/ttyACM0 -b 115200
    ```

3. 启动计算器： 在 `minicom` 的终端中，输入以下命令。

    ```Bash
    rel_cal &
    ```

## 三、自定义应用

您可以通过修改 `demos/relation_calculator/relation_cal.c` 文件来扩展计算器功能，增加新的关系。

此计算器采用状态转移模型来确定最终关系。要增加一个新关系，您需要定义其转换逻辑和对应的名称。

### 1、定义关系逻辑

在 `transitions` 数组中，添加一个新的条目来定义关系如何组合。每个条目遵循 `relation_transformation_t` 结构体格式。

例如，`[我][爸爸]` = `[爸爸]`，这一逻辑被定义为：`{ME, FATHER, FATHER}`。

```C
// 定义关系转换的逻辑
typedef struct relation_transformation_s
{
    relation_type_t from;   // 当前的关系状态
    relation_type_t to;     // 新输入的关系（按下的按钮）
    relation_type_t result; // 最终得到的关系状态
} relation_transformation_t;

// 将新的状态转换逻辑添加到此数组中
static const relation_transformation_t transitions[] = {
    // ... 已有的转换关系 ...
    { ME, FATHER, FATHER }, // 示例：我的爸爸是爸爸
    // 在此处添加您新的转换逻辑
};
```

### 2、定义关系类型与名称

首先，在 `relation_type_e` 枚举中添加一个新的关系类型。然后，在 `relation_names` 数组中添加其对应的显示名称字符串。

**重要提示：** `relation_names` 数组中条目的顺序必须与 `relation_type_e` 枚举中的顺序严格对应，以确保计算器能显示正确的文本。

```C
// 1. 在枚举中添加新的关系类型
typedef enum relation_type_e
{
    // ... 已有的类型 ...
    NEW_RELATIONSHIP, // 您定义的新关系类型
} relation_type_t;

// 2. 在名称数组中添加对应的名称
static const char *relation_names[] = {
    // ... 已有的名称 ...
    "新关系", // NEW_RELATIONSHIP 对应的显示名称
};
```

完成这些修改后，重新构建并部署项目，即可在计算器中看到您添加的新关系。

## 四、实现原理概述

亲戚计算器是基于一个状态转移系统实现的。用户的每一次按键操作都会触发一次状态变更，系统会根据预定义的转换表，将当前关系（如“我”）转换为一个新的关系（如“爸爸”）。该模型能够以清晰且可扩展的方式管理复杂的亲属关系计算。

## 五、贡献与未来改进

当前基于状态转移的实现方式需要预定义大量的静态数据来构建完整的关系图谱。我们正在积极寻求更高效、更具扩展性的模型来优化关系网的构建方式。

如果您对此有更好的实现思路或愿意参与贡献，我们非常欢迎您通过提交 Issue 或 Pull Request 来与社区共同探讨。
