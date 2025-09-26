# Wooden Fish Demo Quick Start

\[ English  | [简体中文](./README_zh-cn.md) \]

This document provides a comprehensive guide on how to build, deploy, and run the Wooden Fish demo application on the OpenVela system for QEMU emulator. You'll learn how to configure the project, compile firmware, run the application, and make custom modifications.

## 1. Demo Features

The Wooden Fish application provides the following features:
- 🐠 **Manual Tapping**: Click the wooden fish image to tap and increase merit points
- 🔄 **Auto Mode**: Enable auto-tapping switch for automatic tapping every 2 seconds
- 💻 **Programmer Mode**: Switch to programmer theme where merit becomes PR (Pull Request)
- 🔊 **Audio Feedback**: Play wooden fish sound effect with each tap
- ✨ **Animation Effects**: Display "Merit+1" or "pr+1" flying text animation when tapping

![Wooden Fish Demo](res/wooden_fish_demo.gif)

## 2. Build and Run

This section will guide you through the complete process from project configuration to running the application on your target platform.

### Prerequisites

Before starting, ensure you are in the root directory of the `openvela` repository. All commands in this document start from this location.

### Step 1: Project Configuration

You need to use the `menuconfig` tool to enable the Wooden Fish application and configure related settings.

1. Launch `menuconfig`. Choose the appropriate command based on your target platform:

    - QEMU Emulator:

        ```Bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
        ```

2. In the `menuconfig` interface, press `/` to open search function, find and enable the following configuration items:

    ```Bash
    LVX_USE_DEMO_WOODEN_FISH=y
    LVX_WOODEN_FISH_DATA_ROOT="/data"
    ```

3. **（Optional）** If you encounter interface lag or display issues during runtime, try increasing the LVGL (Light and Versatile Graphics Library) cache size. Search for `lv_cache_def_size` and set its value to `20000000`.

4. **（Audio Support）** Ensure audio-related configurations are enabled to support wooden fish sound effects:
    ```Bash
    CONFIG_AUDIO=y
    CONFIG_AUDIOUTILS=y
    CONFIG_AUDIOUTILS_NXAUDIO=y
    ```

5. Save configuration and exit `menuconfig`.

### Step 2: Build Project

Before building, it's recommended to clean old build artifacts to avoid potential build errors.

1. Clean build artifacts (distclean):

    - QEMU Emulator:

        ```Bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j8
        ```

2. Execute build:

    - QEMU Emulator:

        ```Bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j8
        ```

### Step 3: Deploy and Run

After successful compilation, you can deploy the firmware to the target platform and launch the application.

#### Option A: Run in QEMU Emulator

1. In the `openvela` root directory, execute the following command to start the emulator:

    ```Bash
    ./emulator.sh vela
    ```

2. After the openvela terminal (`openvela-ap>`) appears, enter the following command to start the Wooden Fish:

    ```Bash
    wooden_fish
    ```

## 3. Application Customization

You can modify the application's core functionality according to your needs, such as replacing resources or adjusting behavior parameters.

### Core File Structure

- `wooden_fish.c`: Contains the core application logic, including UI creation, event handling, animation effects, etc. All feature development and modifications should be done in this file.
- `wooden_fish.h`: Defines application data structures, configuration parameters, and function declarations.
- `wooden_fish_main.c`: Serves as the application entry point, responsible for creating and starting the task that runs the application logic. Usually, you don't need to modify this file.
- `audio_ctl.c` / `audio_ctl.h`: Audio control module responsible for sound effect playback.

### Modifying Resource Files

The images, audio, and font resources used by the application are located in the `res/` directory.

#### Image Resources
- `res/wooden_fish.png`: Traditional wooden fish image
- `res/enter_white.png`: Enter key image for programmer mode

You can replace these image files, but please ensure:
- Image format is PNG
- Resolution is suitable for target display device
- File names remain consistent

#### Audio Resources
- `res/knock.wav`: Tapping sound effect file

To replace sound effects:
1. Prepare WAV format audio file
2. Ensure compatible sample rate and bit depth (recommended 16-bit, 44.1kHz)
3. Replace the `res/knock.wav` file

#### Font Resources
- `res/fonts/MiSans-Normal.ttf`: Font file used by the application

### Adjusting Application Parameters

#### Auto-tapping Frequency
In the `auto_knock_timer_cb` function, auto-tapping interval is controlled by a timer:

```C
// Start auto-tapping timer (every 2 seconds)
app->auto_timer = lv_timer_create(auto_knock_timer_cb, 2000, app);
```

To adjust frequency:
- **Increase frequency**: Decrease time value (e.g., `1500` milliseconds)
- **Decrease frequency**: Increase time value (e.g., `3000` milliseconds)

#### Debounce Time
In the `play_wooden_fish_sound` function, debounce mechanism prevents too-rapid clicking:

```C
// Debounce: only play once within 500ms
if (current_time - last_play_time < 500) {
    return;
}
```

Adjust debounce time to change response sensitivity.

#### Animation Effects
In the `create_merit_animation` function you can adjust flying text animation:

```C
// Adjust animation time and distance
lv_anim_set_values(&move_anim, label_y, label_y - 120);  // Movement distance
lv_anim_set_time(&move_anim, 1500);  // Animation duration
```

## 4. Technical Features

### Audio System
- Uses NuttX nxaudio framework
- Supports WAV format audio playback
- Implements safe resource management to avoid memory leaks

### UI System
- Based on LVGL graphics library
- Responsive layout design
- Smooth animation effects

### State Management
- Singleton pattern for audio resource management
- Debounce mechanism improves user experience
- Unified resource cleanup mechanism

## 5. Troubleshooting

### Common Issues

#### Audio Not Playing
1. Check if audio configuration is correctly enabled
2. Confirm `res/knock.wav` file exists
3. Check if audio device is working properly

#### UI Display Issues
1. Check if font files are loaded correctly
2. Increase LVGL cache size
3. Confirm image resource paths are correct

#### Application Crashes
1. Check if memory configuration is sufficient
2. View error messages in serial output
3. Confirm all dependencies are correctly compiled

### Debugging Methods

Enable debug logging:
```C
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_TRACE
```

View runtime logs for detailed debugging information.

## 6. Contribution Guide

We welcome improvement suggestions and feature enhancements! Before submitting a PR, please ensure:

1. Code style follows project standards
2. Appropriate comments and documentation are added
3. Basic functionality has been tested to work properly
4. Related README documentation has been updated

---

**Note**: This documentation is written based on the OpenVela development environment. If you encounter platform-related issues, please refer to the OpenVela official documentation or submit an Issue in the project repository.
