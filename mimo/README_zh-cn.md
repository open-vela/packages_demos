# MiMo - 小米 AI 大模型接入组件（openvela/NuttX）

面向嵌入式系统的小米 MiMo 大模型接入组件，基于 openvela/NuttX 平台，提供完整的 HTTPS + TLS 通信能力，可直接在设备端调用 MiMo 云端推理服务。

[English](README.md) | 中文

## 功能特性

- 兼容 OpenAI Chat Completions API 协议
- 支持 MiMo 思维链模式（reasoning_content），输出推理过程
- 支持 Function Calling / 工具调用
- 交互式 REPL 和单次命令行两种运行模式
- 基于 mbedTLS 的 HTTPS 安全通信
- 自动管理对话历史，支持多轮对话

## 模型信息

**mimo-v2-flash**：总参数 309B / 激活参数 15B（MoE 混合专家架构），262K 上下文窗口，混合注意力机制。

- API 地址：`https://api.xiaomimimo.com/v1/chat/completions`
- 认证方式：`Bearer <API_KEY>`
- 申请密钥：https://platform.xiaomimimo.com/

## 项目结构

```
packages/demos/mimo/
├── mimo.h             # 核心数据结构与接口定义
├── mimo_main.c        # 主入口，CLI 交互逻辑
├── mimo_agent.c       # Agent 层，管理对话历史与工具调度
├── mimo_provider.c    # Provider 层，HTTP/TLS 通信与 JSON 协议处理
├── Kconfig            # NuttX 菜单配置
├── CMakeLists.txt     # CMake 构建脚本
├── Makefile           # Make 构建脚本
└── Make.defs          # NuttX 应用注册
```

## 编译与运行（模拟器示例）

以下以 openvela `goldfish-armeabi-v7a-ap` 模拟器为例。

### 1. 配置

在 menuconfig 中启用 `DEMOS_MIMO` 并设置 API Key：

```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/ menuconfig
# 找到 Application Configuration → Demos → Xiaomi MiMo AI Provider
# 启用后设置 API Key
```

保存 menuconfig 后，`vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/defconfig` 中将增加以下配置：

```
CONFIG_DEMOS_MIMO=y
CONFIG_MIMO_API_KEY="your_api_key_here"
```

### 配置项说明

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| `MIMO_API_KEY` | "" | MiMo 平台 API 密钥 |
| `MIMO_MODEL` | mimo-v2-flash | 模型标识 |
| `MIMO_API_BASE_URL` | https://api.xiaomimimo.com/v1/chat/completions | API 端点 |
| `MIMO_TEMPERATURE` | 80 | 温度参数 ×100（80 即 0.8） |
| `MIMO_ENABLE_THINKING` | y | 启用思维链推理模式 |
| `MIMO_ENABLE_TOOLS` | y | 启用工具调用 |
| `MIMO_STACKSIZE` | 32768 | 任务栈大小（TLS 需要较大栈空间） |

### 2. 编译

```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j8
```

### 3. 启动模拟器

```bash
./emulator.sh vela -no-windows
```

### 4. 在 NSH 中运行

```bash
# 启动网卡
openvela-ap> ifup eth0
ifup eth0...OK

# 通过 DHCP 获取 IP 地址
openvela-ap> renew eth0

# 启动 MiMo（交互模式）
openvela-ap> mimo

# 或单次问答模式
openvela-ap> mimo 法国的首都是哪里？
```

### 运行示例

```
openvela-ap> mimo
[mimo] Provider initialized
[mimo] Endpoint: https://api.xiaomimimo.com/v1/chat/completions
[mimo] Thinking mode: enabled
[mimo:agent] Initialized (max_turns=10, max_history=32)

  MiMo - Xiaomi AI Assistant
  Model: mimo-v2-flash
  Thinking: on
  Type 'quit' to exit.

You> 你是谁
[mimo] POST https://api.xiaomimimo.com/v1/chat/completions (295 bytes)
[mimo] TLS connecting to api.xiaomimimo.com:443
[mimo] Connecting to api.xiaomimimo.com (120.133.85.111:443)
[mimo] TCP connected, starting TLS handshake
[mimo] TLS handshake complete
[mimo] HTTP 200, response 1433 bytes
[mimo] Thinking: 好的，用户直接问"你是谁"，我需要先确认这是中文的自...
MiMo> 你好！我是MiMo，由小米大模型Core团队开发的AI助手。
      我旨在为用户提供信息查询、问题解答、内容创作、逻辑推理等帮助。
      如果你有任何问题或需要协助，随时告诉我！

You>
```

交互模式下输入 `quit` 或 `exit` 退出。

## 架构设计

```
用户输入 → mimo_main → mimo_agent_chat → mimo_provider_chat → MiMo API
                            ↑                    ↓
                        对话历史管理          HTTP/TLS 通信
                        工具调度循环          JSON 请求构建
                        (ReAct Loop)         响应解析
```

- **Provider 层**（`mimo_provider.c`）：负责 HTTP POST 请求、TLS 握手、JSON 请求体构建和响应解析。使用 NuttX webclient + mbedTLS 实现 HTTPS 通信。
- **Agent 层**（`mimo_agent.c`）：维护对话历史（最多 32 条，自动压缩），实现 ReAct 工具调用循环（最多 10 轮），在思维链模式下保留 `reasoning_content` 用于多轮传递。
- **Main 层**（`mimo_main.c`）：CLI 入口，支持交互式和单次两种模式。

## 推荐采样参数

根据小米官方文档：
- `top_p=0.95`（代码中已固定）
- `temperature=0.8`：适用于数学、写作、Web 开发等场景
- `temperature=0.3`：适用于 Agent / 工具调用场景

## 依赖

- `NETUTILS_CJSON`：JSON 解析
- `NETUTILS_WEBCLIENT`：HTTP 客户端
- `CRYPTO_MBEDTLS`：TLS 加密通信
