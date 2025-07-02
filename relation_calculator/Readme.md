# Relative Calculator

\[ [English](Readme.md) | Simplified Chinese \]

## Running Effect

![alt text](img/show.gif)

## Usage Instructions

### Running on Emulator

#### Project Configuration

1. Switch to the root directory of the openvela repository and execute the following command to configure the relative calculator.

   > The emulator configuration file (defconfig) is located in the `vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/` directory. Use `build.sh` to configure and compile the development board code.

   ```Bash
   ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
   ```

2. Press the `/` key to search and modify the following configuration one by one:

   ```Bash
   LVX_USE_DEMO_RELATIVES_CALCULATOR=y
   ```

#### Compiling the Project
```Bash
# Clean build artifacts
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j$(nproc)

# Start building
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j$(nproc)
```



#### Starting the Calculator
In the emulator's terminal environment `openvela-ap>`, enter the following command:

```Bash
rel_cal &
```

### Running on ESP32S3-box
#### Project Configuration

1. Switch to the root directory of the openvela repository and execute the following command to configure the relative calculator.

   > The emulator configuration file (defconfig) is located in the `nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3` directory. Use `build.sh` to configure and compile the development board code.

   ```Bash
   ./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 menuconfig
   ```

2. Press the `/` key to search and modify the following configuration one by one:

   ```Bash
   LVX_USE_DEMO_RELATIVES_CALCULATOR=y
   ```

#### Compiling the Project

```Bash
# Clean build artifacts
./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 distclean -j$(nproc)

# Start building
./build.sh nuttx/boards/xtensa/esp32s3/esp32s3-box/configs/lvgl-3 -j$(nproc)
```


#### Flashing Resources
Switch to the root directory of the nuttx repository and start flashing resources:

```Bash
make -j20 flash ESPTOOL_PORT=/dev/ttyACM0 ESPTOOL_BINDIR=./
```

#### Starting the Serial Terminal

```Bash
sudo minicom -D /dev/ttyACM0 -b 115200
```

#### Starting the Calculator
In the emulator's terminal environment `openvela-ap>`, enter the following command:

```Bash
rel_cal &
```

## Adding Relationships
If you want to add new relationships, you need to modify the `/demos/relation_calculator/relation_cal.c` file according to the following steps.

### Adding Relationship States
To add a state, simply add a new state to the `static const relation_transformation_t transitions[] = {...}` array, following the format defined by `relation_transformation_t`. The definition of `relation_transformation_t` is as follows:

```C
typedef struct relation_transformation_s
{
    relation_type_t from;
    relation_type_t to;
    relation_type_t result;
} relation_transformation_t;
```

### Adding Supported Relationships
1. Add the new relationship type to `typedef enum relation_type_e`.
2. Add the new relationship name to `static const char *relation_names[]`.

**Note:** Relationship names need to correspond one-to-one with relationship types.

## Implementation Description
The relative calculator is implemented using a state transition approach.

## Current Shortcomings
Both the family tree and state transition approaches inevitably require a large amount of code to build the relationships. Currently, no better implementation method has been thought of. We hope everyone can share better ideas for discussion.