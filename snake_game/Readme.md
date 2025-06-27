# Snake Game

## Introduction

This is an automatic snake game implemented based on the LVGL graphics library. The game area is a 60×35 grid, with each grid cell being 20×20 pixels.

### Features

- Snake automatically seeks and eats food
- Game ends when snake length reaches 1000
- Snake can pass through screen boundaries
- Real-time score display
- Random food generation
- Different colors for snake head (bright green) and body (dark green)
![image](https://github.com/user-attachments/assets/736618e7-0a27-4148-8d9d-9080b5d6e961)

![image](https://github.com/user-attachments/assets/a5b2a22b-0aca-46a8-b874-3b450ce12e3f)


## Configuration

1. Switch to the openvela repository root directory and execute the following command to configure the snake game:

    ```Bash
    ./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
    ```

2. Press `/` key to search and modify the following configurations:

    ```Bash
    LVX_USE_DEMO_SNAKE_GAME=y
    LVX_SNAKE_GAME_DATA_ROOT="/data"
    ```

## Building

```Bash
# Clean build artifacts
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j$(nproc)

# Start building
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j$(nproc)
```

## Running

1. Start the emulator:

    ```Bash
    ./emulator.sh vela
    ```

2. In the emulator terminal environment `openvela-ap>`, enter:

    ```Bash
    snake_game &
    ```

## Game Rules

- Snake starts moving automatically when game launches
- Snake automatically finds shortest path to food
- Score increases by 10 points for each food eaten
- Snake can continue moving by passing through screen boundaries
- Snake automatically changes direction when hitting itself
- Game ends when snake length reaches 1000

## Implementation Details

The game uses a linked list structure to store the snake's body and implements automatic movement through a timer. The snake calculates the shortest path to food (considering boundary crossing) and automatically chooses the optimal movement direction. The game uses LVGL's canvas component for rendering, redrawing the entire screen after each movement.

