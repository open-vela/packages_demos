# Breakout - Brick Breaker Game

English | [中文](README_zh-cn.md)

A classic breakout (brick breaker) game demo built with LVGL on OpenVela/NuttX. Control a paddle to bounce a ball and destroy bricks.

## Features

- Classic breakout gameplay
- Touch/input-based paddle control
- LVGL-based graphics rendering
- libuv event loop integration

## Configuration

Enable in menuconfig:

```
LVX_USE_DEMO_BREAKOUT=y
LVX_BREAKOUT_DATA_ROOT="/data"
LVX_BREAKOUT_STACKSIZE=65536
```

## Project Structure

```
breakout/
├── src/
│   └── breakout.h       # Game logic declarations
├── breakout_main.cpp    # Application entry point (C++)
├── Kconfig              # NuttX configuration
├── CMakeLists.txt       # CMake build script
└── Makefile             # Make build script
```

## Usage

```bash
nsh> breakout
```

## Build

```bash
./build.sh <your-board-config> -j8
```
