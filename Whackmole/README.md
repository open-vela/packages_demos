# Whack-a-Mole Demo Quick Start

\[ English | [简体中文](./README_zh-cn.md) \]

This document provides a detailed guide on how to build, deploy, and run the Whack-a-Mole demo application on the openvela system for both the QEMU emulator and the ESP32-S3-BOX development board. You will learn how to configure the project, compile the firmware, run the application, and customize game features.

## I. Demonstration

![img](img/whackmole.gif)

## II. Build and Run

This section guides you through the entire process, from project configuration to running the application on your target platform.

### Prerequisites

Before you begin, ensure you are in the root directory of the `openvela` repository. All commands in this document are executed from this location.

### Step 1: Configure the Project

You need to use the `menuconfig` utility to enable the Whack-a-Mole application and its related settings.

1. Start `menuconfig`. Select the command corresponding to your target platform:

    - QEMU emulator:

        ```bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
        ```

    - ESP32-S3-BOX:

        ```bash
        ./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 menuconfig
        ```

2. In the `menuconfig` interface, press the `/` key to open the search function, then find and enable the following configuration options:

    ```kconfig
    LVX_USE_DEMO_WHACKMOLE=y
    LVX_WHACKMOLE_DATA_ROOT="/data"
    ```

3. **(Optional)** If you experience UI lag or display issues at runtime, you can increase the cache size for LVGL (Light and Versatile Graphics Library). Search for `lv_cache_def_size` and set its value to `20000000`.

4. Save the configuration and exit `menuconfig`.

### Step 2: Compile the Project

Before compilation, it is recommended to clean the previous build artifacts to prevent potential build errors.

1. Clean build artifacts (distclean):

    - QEMU emulator:

        ```bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j8
        ```

    - ESP32-S3-BOX:

        ```bash
        ./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 distclean
        ```

2. Execute the build:

    - QEMU emulator:

        ```bash
        ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j8
        ```

    - ESP32-S3-BOX:

        ```bash
        ./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 -j8
        ```

### Step 3: Deploy and Run

After a successful compilation, you can deploy the firmware to the target platform and launch the application.

#### Option A: Run on the QEMU Emulator

1. From the `openvela` root directory, execute the following command to start the emulator:

    ```bash
    ./emulator.sh vela
    ```

2. After the openvela terminal (`openvela-ap>`) appears, enter the following command to start the game:

    ```bash
    Whackmole
    ```

#### Option B: Run on the ESP32-S3-BOX Board

1. Flash the firmware:

    Ensure your ESP32-S3-BOX is connected to your computer via USB. Execute the following command to begin flashing. Replace `/dev/ttyACM0` with the actual serial port of your device.

    ```bash
    pushd nuttx && make -j8 flash ESPTOOL_PORT=/dev/ttyACM0 ESPTOOL_BINDIR=./ && popd
    ```

2. Open a serial monitor:

    Use `minicom` or another serial terminal tool to view the device output and interact with it.

    ```bash
    sudo minicom -D /dev/ttyACM0 -b 115200
    ```

3. Start the game:

    In the `minicom` terminal, enter the following command:

    ```bash
    Whackmole
    ```

## III. Application Customization

You can modify the core features of the game, such as replacing resources or adjusting the difficulty, to suit your needs.

### Core File Structure

- `Whackmole.c`: Contains the core game logic. All feature development and modifications should be done in this file.

- `Whackmole_main.c`: Serves as the application entry point, responsible for creating and starting the openvela task that runs the game logic. You typically do not need to modify this file.

### Modify Resource Images

The image and font resources used by the game are converted into C arrays and compiled directly into the firmware.

- **Resource Path**: Resource files are located in the `pic/` directory.

- **Conversion Tool**: You can use the [official LVGL online converter](https://lvgl.io/tools/imageconverter) to convert your own image or font files into the required C format.

- **Replacement**: After generating the new C files, replace the old files in the `pic/` directory and recompile the project.

### Adjust Game Difficulty

The game's difficulty is determined by the frequency at which the moles appear, which is controlled by a timer.

In the `pop_random_mole` function, the game dynamically adjusts the timer's period based on the remaining `game_time`. A shorter period results in moles appearing more frequently, increasing the difficulty.

```c
// Gophers appear randomly
static void pop_random_mole(lv_timer_t *timer) {
    // ... (code omitted) ...
    
    // Adjust the frequency of gophers
    if (game_time < 40) {
        lv_timer_set_period(timer, 800);  // Set the period to 800 ms
    }
    if (game_time < 20) {
        lv_timer_set_period(timer, 600);  // Set the period to 600 ms
    }
}
```

To adjust the game difficulty, you can modify the second argument of the `lv_timer_set_period` function.

- **Decrease Difficulty**: Increase the period value (e.g., to `1000` ms).
- **Increase Difficulty**: Decrease the period value (e.g., to `500` ms).
