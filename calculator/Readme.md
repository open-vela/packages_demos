# 高级计算器 - 基于 open-vela

## 一、基本介绍

基于 open-vela，制作了一个 **高级计算器**，支持以下功能：

### 1. 基本四则运算

- 支持加、减、乘、除运算
- 运算支持 double 精度，仅显示有效数字，不显示多余的 `0`  
  例如：显示 `1.5`，而不是 `1.50000000`

### 2. 高级计算支持

- **取余 (%)**：仅限正整数间操作，错误输入返回 `ERROR!`
- **开平方 (√)**：负数输入将显示错误
- **小数支持**：支持浮点输入与计算，防止重复输入多个小数点
- **删除功能 (Del)**：逐字符删除，支持纠正输入错误
- **清除 (C)** 与 **结果计算 (=)**：
  - `C`：一键清屏
  - `=`：计算表达式，语法错误输出 `ERROR!`

### 3. 结果后输入智能处理

- 运算结果显示后，再次输入新数字会自动清屏
- 如果输入的是运算符，则保留结果并继续运算

---

## 二、实现思路

### 1. 表达式运算处理：`calculate()`

- 实现了一个简易的表达式求值器 `calculate()`，支持从左到右线性计算
- 支持 `+ - * / %` 运算符，解析时跳过空格
- 特别处理了负数输入和非法表达式（通过 `expect_number` 和 `sign` 控制）
- `%` 运算添加了保护机制，仅允许正整数参与，非法输入时返回 `NAN`
- 逻辑简洁高效，适合嵌入式场景中不依赖第三方表达式库的需求

### 2. 基于 LVGL 使用 `create_button()` 动态创建按键

- 通过 `create_button()` 函数动态创建按键
- 使用二维 `btn_map` 配置按钮排布，便于维护和扩展
- 使用 LVGL 的 `lv_btn` 和 `lv_label` 组件实现按钮及输入框，合理设置尺寸和位置

### 3. 核心事件响应机制：`btn_event_cb()`

核心事件回调函数 `btn_event_cb()` 处理所有按钮点击事件：

- 通过 `user_data` 获取按钮内容（数字、运算符、功能键）
- 不同按键分类处理如下：

#### 功能分类处理：

- `C`：清空表达式
- `del`：删除最后一个字符，或清除错误状态
- `sqrt`：计算当前表达式平方根，负数输入时显示 `ERROR!`
- `=`：调用 `calculate()` 执行表达式运算并判断合法性
- `.`：防止连续输入多个小数点
- `%`：仅允许正整数取余，否则返回 `ERROR!`
- 默认输入：自动追加字符，并对错误输入、清屏、输入合法性进行判断

### 4. 清零标志位 `just_calculated`

- 引入 `just_calculated` 状态位，处理“按等号后输入数字会清空”的逻辑
- 如果上次是等号运算，当前输入是数字，则自动清屏
- 如果输入的是运算符，则允许继续使用上次计算结果接着运算

### 5. 输入限制与保护机制

- 表达式字符限制在 64 字节以内，避免溢出
- 输入非法或不符合规则（如多个 `.`、负数开根号、除以零等）时自动显示 `ERROR!`
- 删除、清除、错误重置等操作处理完善，确保系统稳定运行

---

## 📁 原工程路径
vela-opensource/apps/packages/demos/calculator/

---

## 🛠️ 构建与运行指南

### 开始构建

```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j$(nproc)
```
### 配置模拟器（menuconfig）
```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap menuconfig
```
### 清理构建产物
```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean -j$(nproc)
```
### 启动模拟器
```bash
./emulator.sh vela
```

### 启动计算器应用
```bash
calculator &
```

---
