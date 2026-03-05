# Demo Utils

English | [中文](README_zh-cn.md)

Shared utility library for OpenVela/NuttX demo applications. Provides common helper functions used across multiple demo projects.

## Configuration

Enable in menuconfig:

```
LVX_USE_DEMO_UTILS=y
```

This module is automatically selected when enabling demos that depend on it (e.g., Music Player, Music Player 2).

## Build

```bash
./build.sh <your-board-config> -j8
```
