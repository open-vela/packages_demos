# 打地鼠

## 运行效果

![alt text](img/whackmole.gif)

## 使用说明
    Whackmole demo已适配esps3-box开发板，本readme文件都以模拟器为例进行说明。如果想在esp上run，将命令中的`vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap` 全部改为`nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3`即可。其他修改详见文档内容
### 配置项目

1. 切换到 openvela 仓库的根目录，执行如下命令来配置Whackmole。

    模拟器配置文件（defconfig）在 `vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/` 目录下，使用 `build.sh` 配置和编译开发板的代码。

    ```Bash
    ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig //若为esp，执行./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 menuconfig
    ```

2. 按下 `/` 键逐个搜索修改如下配置：

    ```Bash
    LVX_USE_DEMO_WHACKMOLE=y
    LVX_WHACKMOLE_DATA_ROOT="/data"
    ```
3. 如果页面显示不流畅，在menuconfig中将lv_cache_def_size设置为20000000
   
### 编译项目

```Bash
# 模拟器

# 清理构建产物
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j6 #若为esp，执行./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 distclean

# 开始构建
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j6 #若为esp，执行./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 
```
（1）若为qemu：
在nuttx终端执行打开模拟器

```bash
./emulator.sh vela
```
在模拟器输入如下命令启动游戏：
```bash
Whackmole
```

（2）若为esp:
在openvela根目录执行下面命令开始烧录
```bash
pushd nuttx && make -j20 flash ESPTOOL_PORT=/dev/ttyACM0 ESPTOOL_BINDIR=./ && popd
```

打开minicom
```bash
sudo minicom -D /dev/ttyACM0 -b 115200
```
在minicom 中输入如下命令启动游戏：
```bash
Whackmole
```

## 功能修改

首先需要明确，新功能开发时只需要在`Whackmole.c`中增加/修改功能，`Whackmole_main.c`可当作通用开发模板。

### 修改资源图片

资源图片在`pic`下用.c这种内嵌的字体和图片资源烧录到代码中，图片和字体资源可用lvgl官方转换器转换



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


