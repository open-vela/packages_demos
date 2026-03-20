# Bailian (百炼) SDK Adaptation for openvela

本模块实现了阿里云百炼 (Model Studio) 语音交互 SDK 在 openvela goldfish 模拟器上的适配，支持通过 virtio-snd 音频设备完成实时语音对话（ASR → LLM → TTS）。

## 前置条件

1. 百炼 SDK 已放置在 `external/bailian_sdk/` 目录
2. 已在[阿里云百炼控制台](https://bailian.console.aliyun.com/)创建应用，获取以下凭证：
   - **Workspace ID**（工作空间 ID）
   - **App ID**（应用 ID）
   - **API Key**（后付费模式）

## defconfig 配置

在目标板的 defconfig 中添加以下配置项：

### 必选配置

```
# 启用百炼适配模块
CONFIG_AI_BAILIAN=y
CONFIG_AI_BAILIAN_STACKSIZE=32768

# 填入你自己的百炼凭证
CONFIG_AI_BAILIAN_WORKSPACE_ID="your-workspace-id"
CONFIG_AI_BAILIAN_APP_ID="your-app-id"
CONFIG_AI_BAILIAN_API_KEY="your-api-key"

# TLS 支持（WebSocket 连接百炼服务端需要）
CONFIG_CRYPTO_MBEDTLS=y

# 网络（需要 DNS 和路由）
CONFIG_NETDB_DNSSERVER_IPv4=y
CONFIG_NETDB_DNSSERVER_IPv4ADDR=0x08080808
CONFIG_NET_ROUTE=y

# 默认任务栈大小（SDK 内部线程需要较大栈空间）
CONFIG_DEFAULT_TASK_STACKSIZE=16384
```

### 音频相关配置（goldfish 模拟器）

```
# virtio-snd 驱动
CONFIG_DRIVERS_VIRTIO_SOUND=y

# nxaudio 库（播放器依赖）
CONFIG_AUDIOUTILS_NXAUDIO_LIB=y

# ALSA 兼容库
CONFIG_AUDIOUTILS_ALSA_LIB=y
```

### 可选配置

```
# 自定义录音/播放设备路径（默认值通常无需修改）
CONFIG_AI_BAILIAN_RECORDER_DEV="/dev/audio/pcm0c"
CONFIG_AI_BAILIAN_PLAYER_DEV="/dev/audio/pcm0p"
```

> **注意**：修改 defconfig 后需要删除 `cmake_out/` 目录重新编译。

## 编译

```bash
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap --cmake
```

## 运行（goldfish 模拟器）

### 基本启动

```bash
./emulator.sh cmake_out/vela_goldfish-armeabi-v7a-ap -gpu off
```

### 使用主机 USB 麦克风/耳机

通过环境变量指定 PulseAudio 音频设备（不修改 emulator.sh）：

```bash
QEMU_AUDIO_DRV=pa \
  PULSE_SERVER=unix:/run/user/1000/pulse/native \
  QEMU_PA_SOURCE=<your-pa-source-name> \
  QEMU_PA_SINK=<your-pa-sink-name> \
  ./emulator.sh cmake_out/vela_goldfish-armeabi-v7a-ap -gpu off
```

查看可用的 PulseAudio 设备：

```bash
# 查看录音设备（source）
pactl list sources short

# 查看播放设备（sink）
pactl list sinks short
```

### NSH 中操作

```
openvela-ap> ifup eth0        # 启用网络
openvela-ap> renew eth0       # 获取 IP（DHCP）
openvela-ap> bailian          # 启动百炼语音交互
```

启动后按键操作：
- `t` — 开始录音（对着麦克风说话）
- `s` — 停止录音（等待 ASR → LLM → TTS 回复）
- `q` — 退出

## 已知限制

- virtio-snd 不支持 24kHz 采样率，播放端使用 48kHz 并对 TTS 24kHz 音频做 2x 上采样
- 首次录音时 PulseAudio capture 源可能处于 IDLE 状态，代码中已实现 warmup 循环自动激活
- 当前对话延迟约 45 秒，主要瓶颈在 SDK 内部状态机等待，后续版本将优化
