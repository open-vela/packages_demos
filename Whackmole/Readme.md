# 打地鼠

## 运行效果

![alt text](img/whackmole.gif)

## 使用说明

### 配置项目

1. 切换到 openvela 仓库的根目录，执行如下命令来配置Whackmole。

    模拟器配置文件（defconfig）在 `vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/` 目录下，使用 `build.sh` 配置和编译开发板的代码。

    ```Bash
    ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
    ```

2. 按下 `/` 键逐个搜索修改如下配置：

    ```Bash
    LVX_USE_DEMO_WHACKMOLE=y
    LVX_WHACKMOLE_DATA_ROOT="/data"
    ```
3. 如果页面显示不流畅，在menuconfig中将lv_cache_def_size设置为20000000
   
### 编译项目

```Bash
# 清理构建产物
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j6

# 开始构建
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j6
```

### 启动模拟器并推送资源

1. 切换到 openvela 仓库的根目录，启动模拟器：

    ```Bash
    ./emulator.sh vela
    ```

2. 使用模拟器支持的 ADB 将资源推送到设备，在 openvela 仓库的根目录下打开一个新的终端，输入 adb push 后跟文件路径，即可将资源传输到相应位置。

    ```Bash
    # 安装adb
    sudo apt install android-tools-adb

    # 推送资源
    adb push packages/demos/Whackmole/pic /data/

    ```

### 启动游戏

在模拟器的终端环境 `openvela-ap>` 中输入如下命令：

```Bash
Whackmole
```

## 功能修改

首先需要明确，新功能开发时只需要在`Whackmole.c`中增加/修改功能，`Whackmole_main.c`可当作通用开发模板。

### 修改资源图片

在`pic`下的`hammer.png`,`mole.png`,`grassland.png` 对应的位置修改图片资源，并重新推送资源 `adb push packages/demos/Whackmole/pic /data/`

```C
void init_whack_a_mole_game(lv_obj_t *parent) {
    // Set random number seed
    srand(time(NULL));
    R.images.hammer = ICONS_ROOT "/hammer.png";
    R.images.mole = ICONS_ROOT "/mole.png";
    R.images.grassland = ICONS_ROOT "/grassland.png";
    ...
}
```

### 修改游戏难度值

在此游戏中，当游戏时间game_time小于40时，设置地鼠出现的定时器周期为800毫秒；如果小于20，则设为600毫秒。

```C
// Gophers appear randomly
static void pop_random_mole(lv_timer_t *timer) {
    for (int i = 0; i < 9; i++) {
        lv_obj_add_flag(moles[i], LV_OBJ_FLAG_HIDDEN);
    }
    int show_count = rand() % 2 + 1;
    for (int i = 0; i < show_count; i++) {
        int mole_idx = rand() % 9;
        lv_obj_clear_flag(moles[mole_idx], LV_OBJ_FLAG_HIDDEN);
    }
    
    // Adjust the frequency of gophers
    if (game_time < 40) {
        lv_timer_set_period(timer, 800);
    }
    if (game_time < 20) {
        lv_timer_set_period(timer, 600);
    }
}
```
如果我们想要调整游戏难度，可以通过增大或减小定时器周期来实现。


