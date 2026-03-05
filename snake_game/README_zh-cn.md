# 贪吃蛇游戏

[English](README.md) | 中文

## 简介

这是一个使用 LVGL 图形库实现的自动贪吃蛇游戏。游戏区域为 60×35 的网格，每个网格单元为 20×20 像素。

### 功能特性

- 点击 "START" 按钮开始游戏
- 点击开始后，蛇会自动寻找并吃掉食物
- 达到 1000 分即可获胜，显示 "Victory!"
- 撞到自身或墙壁时游戏结束，显示 "GAME OVER"
- 游戏结束时出现 "RESTART" 按钮
- 实时分数显示
- 随机食物生成
- 蛇头为亮绿色，蛇身为深绿色

![模拟器界面](screen/1.png)
![游戏运行界面](screen/2.png)
![游戏开始界面](screen/3.png)
![游戏结束界面](screen/4.png)

## 配置

1. 切换到 openvela 仓库根目录，执行以下命令配置贪吃蛇游戏：

    ```bash
    ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
    ```

2. 按 `/` 搜索并修改以下配置：

    ```bash
    LVX_USE_DEMO_SNAKE_GAME=y
    LVX_SNAKE_GAME_DATA_ROOT="/data"
    ```

## 编译

```bash
# 清理构建文件
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j$(nproc)

# 开始编译
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j$(nproc)
```

## 运行

1. 启动模拟器：

    ```bash
    ./emulator.sh vela
    ```

2. 在模拟器终端环境 `openvela-ap>` 中输入：

    ```bash
    snake_game &
    ```

## 游戏规则

- 点击 "START" 按钮开始游戏
- 蛇会自动寻找到食物的最短路径
- 每吃一个食物得 10 分
- 撞到自身或墙壁时游戏结束，显示 "GAME OVER"
- 游戏结束时点击 "RESTART" 按钮开始新游戏
- 达到 1000 分即可获胜，显示 "Victory!"

## 操作指南

1. **开始游戏**：
   - 点击屏幕中央的 "START" 按钮
   - 游戏开始后按钮自动隐藏

2. **游戏进行中**：
   - 蛇自动寻找食物并移动
   - 左上角显示分数
   - 右上角显示游戏状态

3. **游戏结束**：
   - 蛇撞到自身或墙壁时游戏结束，显示 "GAME OVER"
   - 达到 1000 分获胜，显示 "Victory!"
   - 屏幕中央出现 "RESTART" 按钮
   - 点击 "RESTART" 开始新游戏

## 技术实现

游戏使用链表结构存储蛇身，通过定时器实现自动移动。蛇会计算到食物的最短路径（考虑掉头情况），自动选择最优移动方向。游戏使用 LVGL 的 canvas 组件进行渲染，每次移动后重绘整个屏幕。游戏界面设计简洁，控制按钮和状态显示醒目。
