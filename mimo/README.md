# MiMo - Xiaomi AI Model Provider for openvela/NuttX

A standalone Xiaomi MiMo large language model provider for embedded systems, built on the openvela/NuttX platform with full HTTPS + TLS support for on-device cloud inference.

English | [中文](README_zh-cn.md)

## Features

- OpenAI-compatible Chat Completions API
- Thinking mode (reasoning_content) for chain-of-thought reasoning
- Function calling / tool use support
- Interactive REPL and single-shot CLI modes
- Secure HTTPS communication via mbedTLS
- Automatic conversation history management with multi-turn support

## Model

**mimo-v2-flash**: 309B total / 15B active parameters (MoE architecture), 262K context window, hybrid attention.

- Endpoint: `https://api.xiaomimimo.com/v1/chat/completions`
- Auth: `Bearer <API_KEY>`
- Get your key at: https://platform.xiaomimimo.com/

## Project Structure

```
packages/demos/mimo/
├── mimo.h             # Core data structures and API definitions
├── mimo_main.c        # Entry point, CLI interaction logic
├── mimo_agent.c       # Agent layer: conversation history & tool dispatch
├── mimo_provider.c    # Provider layer: HTTP/TLS communication & JSON handling
├── Kconfig            # NuttX menu configuration
├── CMakeLists.txt     # CMake build script
├── Makefile           # Make build script
└── Make.defs          # NuttX application registration
```

## Build & Run (Emulator Example)

The following example uses the openvela `goldfish-armeabi-v7a-ap` emulator target.

### 1. Configure

Enable `DEMOS_MIMO` in menuconfig and set your API key:

```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/ menuconfig
# Navigate to: Application Configuration → Demos → Xiaomi MiMo AI Provider
# Enable and set API key
```

After saving menuconfig, the following configs will be added to `vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/defconfig`:

```
CONFIG_DEMOS_MIMO=y
CONFIG_MIMO_API_KEY="your_api_key_here"
```

### Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `MIMO_API_KEY` | "" | API key from platform.xiaomimimo.com |
| `MIMO_MODEL` | mimo-v2-flash | Model identifier |
| `MIMO_API_BASE_URL` | https://api.xiaomimimo.com/v1/chat/completions | API endpoint |
| `MIMO_TEMPERATURE` | 80 | Temperature x100 (80 = 0.8) |
| `MIMO_ENABLE_THINKING` | y | Enable chain-of-thought reasoning mode |
| `MIMO_ENABLE_TOOLS` | y | Enable function calling |
| `MIMO_STACKSIZE` | 32768 | Task stack size (32KB recommended for TLS) |

### 2. Build

```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j8
```

### 3. Start Emulator

```bash
./emulator.sh vela -no-windows
```

### 4. Run in NSH

```bash
# Bring up network interface
openvela-ap> ifup eth0
ifup eth0...OK

# Obtain IP address via DHCP
openvela-ap> renew eth0

# Launch MiMo (interactive mode)
openvela-ap> mimo

# Or single-shot mode
openvela-ap> mimo What is the capital of France?
```

### Example Session

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

You> who are you
[mimo] POST https://api.xiaomimimo.com/v1/chat/completions (2318 bytes)
[mimo] TLS connecting to api.xiaomimimo.com:443
[mimo] Connecting to api.xiaomimimo.com (120.133.85.111:443)
[mimo] TCP connected, starting TLS handshake
[mimo] TLS handshake complete
[mimo] HTTP 200, response 1815 bytes
[mimo] Thinking: 好的，用户现在问"who are you"，这是英语版本的"你是谁"...
MiMo> I am MiMo, an AI assistant developed by the Xiaomi LLM Core Team.
      I'm here to help with information queries, problem-solving,
      content creation, logical reasoning, and more.
      How can I assist you today? 😊

You>
```

Type `quit` or `exit` to leave interactive mode.

## Architecture

```
User Input → mimo_main → mimo_agent_chat → mimo_provider_chat → MiMo API
                              ↑                    ↓
                      Conversation History    HTTP/TLS Communication
                      Tool Dispatch Loop      JSON Request Building
                        (ReAct Loop)          Response Parsing
```

- **Provider Layer** (`mimo_provider.c`): Handles HTTP POST requests, TLS handshake, JSON request body construction, and response parsing. Uses NuttX webclient + mbedTLS for HTTPS.
- **Agent Layer** (`mimo_agent.c`): Manages conversation history (up to 32 messages with auto-compaction), implements a ReAct tool-calling loop (up to 10 turns), and preserves `reasoning_content` across multi-turn tool calls in thinking mode.
- **Main Layer** (`mimo_main.c`): CLI entry point supporting both interactive and single-shot modes.

## Recommended Sampling Parameters

Per Xiaomi's documentation:
- `top_p=0.95` (hardcoded)
- `temperature=0.8`: math, writing, web development
- `temperature=0.3`: agentic tasks (tool use)

## Dependencies

- `NETUTILS_CJSON`: JSON parsing
- `NETUTILS_WEBCLIENT`: HTTP client
- `CRYPTO_MBEDTLS`: TLS encryption
