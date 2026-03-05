# Virtual Pet Demo

English | [中文](README_zh-cn.md)

An interactive virtual pet demo application built with LVGL on OpenVela/NuttX. Simulate raising a digital pet by feeding, watering, exercising, and resting to improve its mood and level.

## Features

- Pet status management: hunger, thirst, happiness, experience
- Three mood states: happy, calm, sad (auto-changes based on happiness value)
- Level system: accumulate experience to level up
- Sleep mode: manual toggle, slower status decay while sleeping
- Achievement system: consecutive feeding/watering/exercise streaks, survival days, level milestones
- Data persistence: auto-save and restore on restart
- Modern UI: left status panel, right action buttons, graphical interaction

## Configuration

Enable in menuconfig:

```
LVX_USE_DEMO_PET=y
LVX_PET_DATA_ROOT="/sdcard"
```

## Project Structure

```
pet/
├── pet.h           # Data structures, constants, and API definitions
├── pet.c           # Core functionality implementation
├── pet_main.c      # Application entry point and LVGL initialization
├── Kconfig         # NuttX configuration
├── CMakeLists.txt  # CMake build script
└── Makefile        # Make build script
```

## Resource Structure

```
<DATA_ROOT>/res/
├── fonts/    # Font files
├── image/    # Pet and background images
└── icons/    # Action button icons
```

## Usage

```bash
nsh> pet
```

- Feed: tap the food icon to increase hunger
- Water: tap the cup icon to increase thirst
- Exercise: tap the exercise icon to boost happiness and gain experience
- Sleep: tap the sleep icon to toggle sleep mode

## Build

```bash
./build.sh <your-board-config> -j8
```
