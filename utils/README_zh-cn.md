# Demo Utils - 演示工具库

[English](README.md) | 中文

OpenVela/NuttX 演示应用的共享工具库，提供多个演示项目共用的辅助函数。

## 配置

在 menuconfig 中启用：

```
LVX_USE_DEMO_UTILS=y
```

当启用依赖此模块的演示应用（如音乐播放器、音乐播放器 2）时，会自动选中此模块。

## 编译

```bash
./build.sh <your-board-config> -j8
```
