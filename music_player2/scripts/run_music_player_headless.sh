#!/usr/bin/env bash
set -euo pipefail

# Vela Music Player Headless Launch Script

# Resolve repo root
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
DEFCONFIG="$ROOT_DIR/vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/defconfig"

echo "[run] repo root: $ROOT_DIR"

# Configure defconfig
ensure_cfg() {
    local line="$1"
    local key="${line%%=*}"
    sed -i "/^${key}=.*/d" "$DEFCONFIG" || true
    echo "$line" >> "$DEFCONFIG"
}

echo "[run] Updating defconfig for music_player and NSH builtins..."
ensure_cfg "CONFIG_BUILTIN=y"
ensure_cfg "CONFIG_NSH_LIBRARY=y"
ensure_cfg "CONFIG_NSH_BUILTIN_APPS=y"
ensure_cfg "CONFIG_SYSTEM_NSH=y"
ensure_cfg "CONFIG_LVX_USE_DEMO_VELA_AUDIO=y"
ensure_cfg "CONFIG_LVX_MUSIC_PLAYER_DATA_ROOT=\"/data\""

# Enable MP3 support and audio features
ensure_cfg "CONFIG_LVX_MUSIC_PLAYER_MP3_SUPPORT=y"
ensure_cfg "CONFIG_LIB_MAD=y"
ensure_cfg "CONFIG_AUDIO=y"
ensure_cfg "CONFIG_AUDIO_DEVICES=y"
ensure_cfg "CONFIG_DRIVERS_AUDIO=y"

# Enable LVGL support
ensure_cfg "CONFIG_LVGL=y"
ensure_cfg "CONFIG_LV_USE_DEMO_MUSIC=y"

# Disable GUI features for headless mode
ensure_cfg "# CONFIG_LV_USE_SDL is not set"
ensure_cfg "# CONFIG_LV_USE_FBDEV is not set"

echo "[run] Building..."
cd "$ROOT_DIR/nuttx"
./tools/configure.sh vela/goldfish-armeabi-v7a-ap
make -j$(nproc)

echo "[run] Starting headless emulator..."
cd "$ROOT_DIR"
exec ./tools/emulator.sh vela/goldfish-armeabi-v7a-ap -nographic