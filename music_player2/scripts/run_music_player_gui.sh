#!/usr/bin/env bash
set -euo pipefail

# Vela Music Player GUI Launch Script
# 
# Usage:
#   ./run_music_player_gui.sh           # Normal startup
#   ./run_music_player_gui.sh --rebuild # Rebuild and start
#   ./run_music_player_gui.sh --help    # Show help

# Show help
if [ "${1:-}" = "--help" ] || [ "${1:-}" = "-h" ]; then
    echo "Vela Music Player GUI Launch Script"
    echo ""
    echo "Usage:"
    echo "  $0 [options]"
    echo ""
    echo "Options:"
    echo "  --rebuild, -r    Rebuild mode: clean build artifacts and recompile"
    echo "  --help, -h       Show this help"
    echo ""
    echo "Features:"
    echo "  - Auto architecture detection"
    echo "  - Rebuild support"
    echo "  - Auto emulator management"
    echo "  - Resource file push"
    echo ""
    exit 0
fi

# Resolve repo root
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
DEFCONFIG="$ROOT_DIR/vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/defconfig"

echo "[run-gui] repo root: $ROOT_DIR"

# Rebuild function
rebuild_music_player() {
    echo "Starting music player rebuild..."
    
    # Clean music player build artifacts
    echo "[rebuild] Cleaning music player build artifacts..."
    MUSIC_DIR="$ROOT_DIR/apps/packages/demos/music_player"
    if [ -d "$MUSIC_DIR" ]; then
        find "$MUSIC_DIR" -name "*.o" -delete 2>/dev/null || true
        find "$MUSIC_DIR" -name "*.d" -delete 2>/dev/null || true
        find "$MUSIC_DIR" -name ".built" -delete 2>/dev/null || true
        find "$MUSIC_DIR" -name ".depend" -delete 2>/dev/null || true
        rm -f "$MUSIC_DIR/Make.dep" 2>/dev/null || true
        echo "Music player build artifacts cleaned"
    fi
    
    # Clean builtin registry
    echo "[rebuild] Cleaning builtin registry..."
    find "$ROOT_DIR/apps/builtin/registry" -name "*music_player*" -delete 2>/dev/null || true
    
    # Force reconfigure
    echo "[rebuild] Force reconfigure..."
    cd "$ROOT_DIR/nuttx"
    make distclean >/dev/null 2>&1 || true
    
    echo "Music player rebuild complete, ready to recompile..."
}

# Check for rebuild flag
if [ "${1:-}" = "--rebuild" ] || [ "${1:-}" = "-r" ]; then
    echo "Rebuild mode detected, executing rebuild..."
    rebuild_music_player
    shift
fi

# System requirements check
echo "[run-gui] System environment check..."
check_failed=false

# Check required tools
for tool in gcc make adb; do
    if ! command -v $tool >/dev/null 2>&1; then
        echo "Missing required tool: $tool"
        check_failed=true
    else
        echo "Found: $tool"
    fi
done

if [ "$check_failed" = true ]; then
    echo "System check failed. Please install missing tools."
    exit 1
fi

# Configure defconfig
ensure_cfg() {
    local line="$1"
    local key="${line%%=*}"
    sed -i "/^${key}=.*/d" "$DEFCONFIG" || true
    echo "$line" >> "$DEFCONFIG"
}

echo "[run-gui] Updating defconfig for music_player..."
ensure_cfg "CONFIG_BUILTIN=y"
ensure_cfg "CONFIG_NSH_LIBRARY=y"
ensure_cfg "CONFIG_NSH_BUILTIN_APPS=y"
ensure_cfg "CONFIG_SYSTEM_NSH=y"
ensure_cfg "CONFIG_LVX_USE_DEMO_VELA_AUDIO=y"
ensure_cfg "CONFIG_LVX_MUSIC_PLAYER_DATA_ROOT=\"/data\""
ensure_cfg "CONFIG_LVX_MUSIC_PLAYER_MP3_SUPPORT=y"
ensure_cfg "CONFIG_LIB_MAD=y"
ensure_cfg "CONFIG_AUDIO=y"
ensure_cfg "CONFIG_AUDIO_DEVICES=y"
ensure_cfg "CONFIG_DRIVERS_AUDIO=y"
ensure_cfg "CONFIG_LVGL=y"
ensure_cfg "CONFIG_LV_USE_DEMO_MUSIC=y"

# Auto-detect architecture
detect_arch() {
    for arch in armeabi-v7a-ap arm64-ap x86_64-ap; do
        DEFCONFIG_ARCH="$ROOT_DIR/vendor/openvela/boards/vela/configs/goldfish-${arch}/defconfig"
        if [ -f "$DEFCONFIG_ARCH" ]; then
            echo "$arch"
            return 0
        fi
    done
    echo "armeabi-v7a-ap"  # fallback
}

ARCH=$(detect_arch)
echo "[run-gui] Using architecture: $ARCH"

# Build
echo "[run-gui] Building music player..."
cd "$ROOT_DIR/nuttx"

# Configure
./tools/configure.sh "vela/goldfish-${ARCH}"

# Build
make -j$(nproc) || {
    echo "Build failed"
    exit 1
}

echo "[run-gui] Build completed successfully!"

# Start emulator
echo "[run-gui] Starting emulator..."
cd "$ROOT_DIR"

# Kill existing emulator
pkill -f "qemu-system" || true
sleep 2

# Start new emulator
nohup ./tools/emulator.sh vela/goldfish-${ARCH} > emulator.log 2>&1 &
EMULATOR_PID=$!
echo "[run-gui] Emulator started (PID: $EMULATOR_PID)"

# Wait for emulator to be ready
echo "[run-gui] Waiting for emulator to start..."
sleep 10

# Wait for ADB connection
echo "[run-gui] Waiting for ADB connection..."
timeout=30
while [ $timeout -gt 0 ]; do
    if adb shell echo "ready" >/dev/null 2>&1; then
        echo "[run-gui] ADB connection established"
        break
    fi
    sleep 2
    timeout=$((timeout - 2))
done

if [ $timeout -le 0 ]; then
    echo "ADB connection timeout"
    exit 1
fi

# Push music resources
echo "[run-gui] Pushing music resources..."
MUSIC_RES_DIR="$ROOT_DIR/music_player/res"
if [ -d "$MUSIC_RES_DIR" ]; then
    adb shell mkdir -p /data/music /data/icons
    adb push "$MUSIC_RES_DIR"/* /data/ || true
    echo "[run-gui] Music resources pushed"
fi

# Launch music player
echo "[run-gui] Launching music player..."
adb shell music_player &

echo "[run-gui] Music player GUI launched successfully!"
echo "[run-gui] Emulator log: $ROOT_DIR/emulator.log"
echo "[run-gui] Use 'adb shell' to connect to the device"