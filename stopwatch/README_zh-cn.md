# 秒表

\[ [English](README.md) | 简体中文 \]

## 概述

一个基于 LVGL 构建的简易秒表应用，运行在 OpenVela 平台上。提供基本的计时功能，支持记录分段时间。

## 功能特性

- 开始 / 停止 / 重置控制
- 分段计时记录（最多 10 个分段）
- 可滚动的分段历史列表
- 约 30 fps 的实时显示刷新
- 简洁的深色主题 UI

## 构建

在 NuttX 配置中启用该 demo：

```
CONFIG_LVX_USE_DEMO_STOPWATCH=y
```

然后按照常规方式构建项目。

## 使用方法

在 NuttX shell 中运行：

```shell
nsh> stopwatch
```

- 点击 **Start** 开始计时
- 计时过程中点击 **Lap** 记录分段时间
- 点击 **Stop** 暂停计时
- 停止状态下点击 **Reset** 清除所有数据

## 文件结构

```
stopwatch/
├── CMakeLists.txt       # CMake 构建配置
├── Kconfig              # NuttX Kconfig 配置选项
├── Make.defs            # NuttX 构建系统集成
├── Makefile             # NuttX Makefile
├── README.md            # 英文文档
├── README_zh-cn.md      # 本文件
├── stopwatch_main.c     # 应用入口
├── stopwatch_ui.c       # UI 创建与事件处理
└── stopwatch_ui.h       # UI 头文件
```

## 依赖

- LVGL (v9.x)
- libuv
- NuttX RTOS

## 许可证

Apache License 2.0
