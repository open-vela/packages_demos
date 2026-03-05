English | [中文](README_zh-cn.md)

# Advanced Calculator - Based on open-vela

## 📚 Table of Contents

- [1. Introduction](#1-introduction)
- [2. Implementation](#2-implementation)
- [3. Usage Guide](#3-usage-guide)
- [📁 Project Path](#project-path)
- [🛠️ Common Commands](#common-commands)

---

## 1. Introduction

- An **advanced calculator** built on open-vela.
- The expression calculation engine is based on the repository https://github.com/W-Mai/ExpressionCalc
- `calculate` computes the result, `Clear` clears the screen, `Del` deletes the last character.
- Functions like sqrt and cos require parentheses; constants PI and E can be used directly.
  For example: PI / 2, E + 4, sqrt(4), cos(-2)

![Calculator UI](./screen_history/screenshot_commit7.png)

The calculator supports the following features:

### 1.1 Basic Arithmetic

- Addition, subtraction, multiplication, and division
- Double precision arithmetic, displaying only significant digits (no trailing zeros)
- Example: displays `1.5` instead of `1.50000000`

### 1.2 Advanced Functions
Usage example: cos(2)

- sqrt — Square root
- log — Natural logarithm (ln)
- sin — Sine function
- cos — Cosine function
- PI — Pi constant (3.1415926...)
- E — Euler's number (2.7182818...)
- . — Decimal point
- ( — Left parenthesis
- ) — Right parenthesis

### 1.3 Smart Input After Result

- After a result is displayed, entering a new number automatically clears the screen
- If an operator is entered, the result is retained and computation continues

---

## 2. Implementation

### 2.1 Token Levels and Operator Precedence

- Operator priority is defined via TokenLevel. For example, + and - have priority 1, while *, /, % have priority 2.
- This enables correct operator precedence when processing reverse Polish notation.

### 2.2 Expression Parsing
- The `reversePolishNotation` method converts infix expressions (e.g., `a + b * c`) to reverse Polish notation (e.g., `a b c * +`) using a stack to handle operators and parentheses.
- Numbers are added directly to the result queue.
- Operators are pushed/popped based on their precedence.
- Left parentheses are pushed onto the stack; right parentheses pop operators until a left parenthesis is found.

### 2.3 Reverse Polish Notation Evaluation

- The `evalNotation` method evaluates reverse Polish notation using a stack.
- Numbers are pushed onto the stack.
- When an operator is encountered, operands are popped, computed, and the result is pushed back.
- Supports both custom and built-in operators: +, -, *, sin, log, etc.

### 2.4 Dynamic Button Creation with LVGL `create_button()`

- Buttons are dynamically created via the `create_button()` function.
- A 2D `btn_map` configures button layout for easy maintenance and extension.
- LVGL component functions set button and input field sizes, positions, and colors.

### 2.5 Input Protection

- Invalid input (e.g., multiple `.`, square root of negative numbers, division by zero) displays `ERROR`.
- Delete, clear, and error reset operations are fully handled to ensure stable operation.

---

## 3. Usage Guide

### 3.1 Configure the Emulator (menuconfig)
```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
```

#### (1) Build Settings
- Set `LVX_USE_DEMO_CALCULATOR` to `yes`
- Set `LVX_CALCULATOR_DATA_ROOT` to `/data` (default in Kconfig)

#### (2) C++ Header Configuration
To support C++ headers (e.g., iostream.h, cmath.h):
- Set C++ library to `Toolchain C++ support`
- Set C++ low level library select to `GNU low level libsupc++`
- Set Language standard to `(gnu++20)`

For try-catch exception handling (not recommended for embedded, not used in current version):
- Enable `exception support`

### 3.2 Makefile Build Issues

#### (1) Main Function Not Found
```
arm-none-eabi-ld: ... undefined reference to calculator_main'
```
- Because the Makefile sets `PROGNAME = calculator`, the linker expects a function named `int calculator_main(int argc, char *argv[])`.
- You must also add `extern "C"`, so the main function should be `extern "C" int calculator_main` to compile successfully.

#### (2) Makefile Configuration
- Since C++ files are used, add `CXXEXT` to specify .cpp files:
```
CXXEXT := .cpp
```
- C++ source files must be listed under `CXXSRCS`:
```
CXXSRCS = calculator_cre.cpp expression_calc.cpp
```

### 3.3 Resource Management

#### (1) Built-in Resource Mode (Recommended, enabled by default)
Enable `use the builtin resources` in menuconfig (default `y`):
- Fonts and images are compiled into the firmware, no additional push needed
- ELF size increases
- Pros: Simple deployment, no adb push required
- Use case: Production, quick testing

#### (2) File System Resource Mode
Disable `use the builtin resources` in menuconfig (set to `n`):
- Resources must be pushed via ADB while the emulator is running:

Start the emulator:
```bash
./emulator.sh vela
```

Push resources via ADB:
```bash
adb push apps/packages/demos/calculator/res /data/
```

Launch the calculator:
```bash
calculator &
```

- Pros: Dynamically replaceable resources, smaller ROM footprint
- Use case: Development, frequent UI resource changes

## 📁 Project Path
vela-opensource/apps/packages/demos/calculator/

---

## 🛠️ Common Commands

### Build
```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j$(nproc)
```

### Configure Emulator (menuconfig)
```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
```

### Push Resources via ADB (file system resource mode, emulator running)
```bash
adb push apps/packages/demos/calculator/res /data/
```

### Clean Build Artifacts
```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j$(nproc)
```

### Start Emulator
```bash
./emulator.sh vela
```

### Launch Calculator
```bash
calculator &
```