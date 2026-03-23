# 天气仪表盘

\[ [English](README.md) | 简体中文 \]

## 概述

一个功能丰富的天气仪表盘应用，基于 LVGL 构建，运行在 OpenVela 平台上。展示了多页面导航、实时数据可视化、天气动画效果和交互式设置等功能，全部运行在 NuttX RTOS 上。

## 功能特性

- **多页面导航**：带有滑动 + 淡入动画的页面切换
- **主页**：当前天气及动画效果（雨滴/雪花/阳光光晕）、24小时温湿度折线图、7天天气预报
- **详情页**：温度/湿度/气压/AQI 圆弧仪表盘、UV 指数条、风速风向显示
- **设置页**：温度单位切换（°C/°F）、城市选择下拉框、刷新间隔滑块、关于信息
- **模拟传感器数据**：带有真实波动的演示数据
- **深色主题 UI**：统一的配色方案
- **底部导航栏**：图标 + 文字按钮
- **状态栏**：显示城市名称和时钟

## 架构设计

```
weather_dashboard/
├── CMakeLists.txt                  # CMake 构建配置
├── Kconfig                         # NuttX 配置选项
├── Make.defs                       # NuttX 构建系统集成
├── Makefile                        # NuttX Makefile
├── README.md                       # 英文文档
├── README_zh-cn.md                 # 中文文档
├── weather_dashboard_main.c        # 应用入口、导航、定时器
└── src/
    ├── weather_data.h              # 数据模型定义
    ├── weather_data.c              # 模拟天气数据引擎
    ├── weather_ui.h                # UI 定义、配色、页面接口
    ├── page_home.c                 # 主页（概览 + 图表 + 预报）
    ├── page_detail.c               # 详情页（仪表盘 + UV + 风速）
    ├── page_settings.c             # 设置页（单位/城市/刷新）
    ├── chart_widget.c              # 可复用的 24h 折线图组件
    ├── gauge_widget.c              # 可复用的圆弧仪表盘组件
    └── weather_anim.c              # 天气条件粒子动画
```

## 设计模式

- **MVC 分离**：`weather_data`（模型）、`page_*`（视图）、`weather_dashboard_main`（控制器）
- **页面生命周期**：每个页面实现 `create()`、`update()`、`destroy()` 回调
- **可复用组件**：`chart_widget` 和 `gauge_widget` 是自包含组件，通过 `lv_obj_set_user_data()` 管理私有数据
- **动画系统**：基于粒子的天气效果（雨滴、雪花、阳光光晕），使用 LVGL 动画 API

## 构建

在 NuttX 配置中启用：

```
CONFIG_LVX_USE_DEMO_WEATHER_DASHBOARD=y
```

可选配置：
```
CONFIG_WEATHER_DASHBOARD_CITY_NAME="Beijing"
CONFIG_WEATHER_DASHBOARD_TEMP_UNIT=0    # 0=摄氏度, 1=华氏度
```

## 使用方法

```shell
nsh> weather_dashboard
```

- 通过底部导航栏切换页面
- 主页：查看当前天气、小时图表和每周预报
- 详情：实时监控环境仪表盘
- 设置：切换单位、更改城市、调整刷新频率

## 依赖

- LVGL (v9.x)，需要 chart 支持
- libuv
- NuttX RTOS

## 许可证

Apache License 2.0
