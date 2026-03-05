English | [中文](README_zh-cn.md)

# Hourglass Demo

## Overview
A digital hourglass demo application running on the OpenVela system emulator. It uses the LVGL graphics library to create a visual hourglass effect, where users can control the hourglass start and timer duration via touchscreen.

## Features
- Realistic hourglass animation: simulates sand particles flowing from top to bottom.
- Interactive controls:
  - Start/Reset: start or reset the hourglass timer.
  - Time adjustment: increase or decrease the total timer duration.
- Modular design: UI and control logic are separated for easy maintenance and extension.

## Prerequisites
- OpenVela development environment set up (Reference: [Environment Setup](https://gitee.com/open-vela/docs/blob/dev/zh-cn/quickstart/Set_up_the_development_environment_zh-cn.md))
- OpenVela source code obtained (Reference: [Download OpenVela Source](https://gitee.com/open-vela/docs/blob/dev/zh-cn/quickstart/Download_Vela_sources_zh-cn.md))

## Getting Started

### 1. Configure the Project
```bash
./build.sh vendor/openvela/boards/vela/configs/qemu-armeabi-v7a-ap/ distclean -j$(nproc) menuconfig
```
In the configuration interface:
- Press `/` to search for `LVX_USE_DEMO_HOURGLASS`.
- Press Enter to navigate to the option.
- Press Space to mark it as `[*]` (i.e., `=y`) to enable the demo.
- Press `Q` multiple times to exit, and press `Y` when prompted to save.

Note: Other related configurations (device path, task priority, etc.) are pre-configured and do not need modification unless you have special requirements.

### 2. Build
```bash
./build.sh vendor/openvela/boards/vela/configs/qemu-armeabi-v7a-ap/ -j$(nproc)
```

Clean build artifacts:
```bash
./build.sh vendor/openvela/boards/vela/configs/qemu-armeabi-v7a-ap/ distclean -j$(nproc)
```

### 3. Run the Demo
Start the emulator:
```bash
./emulator.sh vela
```

Launch the hourglass application:
```bash
hourglass &
```

## Project Structure
```
vela/packages/demos/hourglass/
├── hourglass_page.c/h    # UI rendering, button event handling
├── hourglass_control.c/h # Core hourglass animation control logic and state machine
├── hourglass_main.c      # Program entry point, task initialization
├── Kconfig               # Configuration options (enable, priority, device path, etc.)
├── Make.defs             # Build system dependency definitions
└── Makefile              # Build rules
```

## Core Implementation

### UI
- Hourglass body: Composed of two 45-degree rotated square containers, each dynamically creating an 8x8 matrix of red squares to simulate sand particles.
- Control panel: Contains Start button and +/- buttons for controlling the hourglass and adjusting time.

### Control Logic
- State machine: Manages three states — initial, running, and finished.
- Timer-driven: Core animation is periodically triggered by an LVGL timer.
- Sand movement algorithm: Uses diagonal scanning with random direction strategy, moving one "sand particle" (changing a square's color) at a time to simulate natural falling.

## Troubleshooting
- Demo not starting: Verify that `LVX_USE_DEMO_HOURGLASS` is correctly set to `y` in the configuration step.
- Touch not responding: Check that the device path configured in `LVX_USE_DEMO_HOURGLASS_DEVPATH` matches the actual system device node.
- Build errors: Ensure the development environment is properly set up and a thorough `distclean` has been performed.