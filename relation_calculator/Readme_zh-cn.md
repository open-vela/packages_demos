# 亲戚计算器

\[ [English](Readme.md) | 简体中文 \]

## 运行效果

![alt text](img/show.gif)

## 使用说明

### 模拟器运行

#### 配置项目

1. 切换到 openvela 仓库的根目录，执行如下命令来配置亲戚计算器。

    > 模拟器配置文件（defconfig）在 `vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/` 目录下，使用 `build.sh` 配置和编译开发板的代码。

    ```Bash
    ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
    ```

2. 按下 `/` 键逐个搜索修改如下配置：

    ```Bash
    LVX_USE_DEMO_RELATIVES_CALCULATOR=y
    ```

#### 编译项目
```Bash
# 清理构建产物
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j$(nproc)

# 开始构建
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j$(nproc)
```

#### 启动计算器
在模拟器的终端环境 `openvela-ap>` 中输入如下命令：

```Bash
rel_cal &
```

### ESP32S3-box 运行
#### 配置项目

1. 切换到 openvela 仓库的根目录，执行如下命令来配置亲戚计算器。

    > 模拟器配置文件（defconfig）在 `nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3` 目录下，使用 `build.sh` 配置和编译开发板的代码。

    ```Bash
    ./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 menuconfig
    ```

2. 按下 `/` 键逐个搜索修改如下配置：

    ```Bash
    LVX_USE_DEMO_RELATIVES_CALCULATOR=y
    ```

#### 编译项目

```Bash
# 清理构建产物
./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 distclean -j$(nproc)

# 开始构建
./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 -j$(nproc)
```

#### 烧录资源
切换到 nuttx 仓库的根目录，开始烧录资源：

```Bash
make -j20 flash ESPTOOL_PORT=/dev/ttyACM0 ESPTOOL_BINDIR=./
```

#### 启动串口终端

```Bash
sudo minicom -D /dev/ttyACM0 -b 115200
```

#### 启动计算器
在模拟器的终端环境 `openvela-ap>` 中输入如下命令：

```Bash
rel_cal &
```

## 增加关系
如果想要增加新的关系，需要在 `/demos/relation_calculator/relation_cal.c` 文件中根据以下步骤进行修改。

### 增加关系状态
添加状态只需要在 `static const relation_transformation_t transitions[] = {...}` 数组中添加新的状态,格式需要按照 `relation_transformation_t` 的定义来填写。
`relation_transformation_t` 的定义如下：

```C
typedef struct relation_transformation_s
{
    relation_type_t from;
    relation_type_t to;
    relation_type_t result;
} relation_transformation_t;
```

### 增加支持关系
1. 在 `typedef enum relation_type_e` 中添加新的关系类型。
2. 在 `static const char *relation_names[]` 中添加新的关系名称。

**注意：** 关系名称需要和关系类型一一对应。

## 实现说明
采用了状态转移的方式来实现这个亲戚计算器。


## 目前缺点
家族树和状态转移的方式始终绕不开需要使用大量的代码来构建这个关系，目前想不到更好的实现方式了。希望大家有更好的想法可以一起讨论。

