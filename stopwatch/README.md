# Stopwatch

\[ English | [简体中文](README_zh-cn.md) \]

## Overview

A simple stopwatch application built with LVGL for the OpenVela platform. It provides basic timing functionality with lap recording support.

## Features

- Start / Stop / Reset controls
- Lap time recording (up to 10 laps)
- Scrollable lap history list
- Real-time display update at ~30 fps
- Clean dark-themed UI

## Screenshots

| Idle | Running | Laps |
|------|---------|------|
| ![idle](screenshots/idle.png) | ![running](screenshots/running.png) | ![laps](screenshots/laps.png) |

> Screenshots are placeholders. Replace with actual device captures.

## Build

Enable the demo in your NuttX configuration:

```
CONFIG_LVX_USE_DEMO_STOPWATCH=y
```

Then build the project as usual.

## Usage

Run the application from the NuttX shell:

```shell
nsh> stopwatch
```

- Press **Start** to begin timing.
- Press **Lap** while running to record a lap time.
- Press **Stop** to pause the timer.
- Press **Reset** (when stopped) to clear all data.

## File Structure

```
stopwatch/
├── CMakeLists.txt       # CMake build configuration
├── Kconfig              # NuttX Kconfig options
├── Make.defs            # NuttX build system integration
├── Makefile             # NuttX Makefile
├── README.md            # This file
├── README_zh-cn.md      # Chinese documentation
├── stopwatch_main.c     # Application entry point
├── stopwatch_ui.c       # UI creation and event handling
└── stopwatch_ui.h       # UI header file
```

## Dependencies

- LVGL (v9.x)
- libuv
- NuttX RTOS

## License

Apache License 2.0
