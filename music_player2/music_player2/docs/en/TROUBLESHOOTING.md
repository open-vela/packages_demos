# Vela Audio Troubleshooting Guide

This document provides common problem solutions, debugging techniques, and performance optimization suggestions for the Vela Audio music player.

## Table of Contents

- [Common Issues](#common-issues)
- [Debugging Techniques](#debugging-techniques)
- [Performance Optimization](#performance-optimization)

## Common Issues

### 1. Compilation Errors

#### Issue: `music_player: command not found`
**Symptoms**: Cannot find music_player command after compilation

**Solution**:
```bash
# Check if configuration is correct
grep -r LVX_USE_DEMO_MUSIC_PLAYER vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap/defconfig

# Reconfigure and compile
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap distclean
./build.sh vendor/openvela/boards/vela/configs/goldfish-armeabi-v7a-ap -j8
```

#### Issue: Stack overflow error
**Symptoms**: Stack overflow crash during runtime

**Solution**:
```bash
# Edit Makefile, adjust stack size
STACKSIZE = 32768  # Currently 16384, can be adjusted to 32768
```

### 2. Runtime Errors

#### Issue: Resource files not found
**Symptoms**: Application starts with default interface, cannot load music or icons

**Solution**:
```bash
# Check if files exist
adb shell ls -la /data/res/

# Re-push resources
adb push res/ /data/
```

#### Issue: Audio playback failure
**Symptoms**: No response when clicking play button, or abnormal playback status

**Solution**:
```bash
# Check audio device
adb shell ls -la /dev/audio/

# Check audio format
file res/musics/*.mp3

# Verify audio file integrity
adb shell "cd /data/res/musics && ls -la *.mp3"
```

### 3. Interface Issues

#### Issue: Abnormal interface display
**Symptoms**: Garbled font display, interface layout disorder, abnormal colors

**Troubleshooting Steps**:
- Check if LVGL 9.x configuration is correct
- Ensure font files are loaded correctly
- Check screen resolution settings
- Verify resource file paths

**Solution**:
```bash
# Check font files
adb shell ls -la /data/res/fonts/

# Re-push font resources
adb push res/fonts/ /data/res/fonts/
```

#### Issue: Unresponsive operations
**Symptoms**: Touch operations have no effect, button clicks are unresponsive

**Troubleshooting Steps**:
- Check if touchscreen driver is working properly
- Look for error messages in logs
- Ensure application has initialized correctly
- Verify event handler functions are registered correctly

### 4. Emulator Issues

#### Issue: Emulator startup failure
**Symptoms**: Executing `./emulator.sh vela` fails

**Solution**:
```bash
# Check nuttx file
ls -la nuttx/nuttx

# Recreate symbolic link
cd nuttx && rm -f nuttx && ln -sf vela_ap.elf nuttx && cd ..

# Ensure build artifacts exist
ls -la nuttx/vela_ap.elf
```

#### Issue: ADB connection failure
**Symptoms**: Cannot connect to emulator, adb connect fails

**Solution**:
```bash
# Check port usage
netstat -tlnp | grep 555

# Restart ADB service
adb kill-server
adb start-server

# Reconnect
adb disconnect 127.0.0.1:5555
sleep 2
adb connect 127.0.0.1:5555

# Verify connection
adb devices
```

## Debugging Techniques

### 1. Using Log Output

Add debug logs in code to track program execution:

```c
// Define debug macro at the beginning of source file
#define MUSIC_DEBUG 1

#if MUSIC_DEBUG
#define DLOG(fmt, ...) printf("[MUSIC] " fmt "\n", ##__VA_ARGS__)
#else
#define DLOG(fmt, ...)
#endif

// Add logs at key positions
DLOG("Loading audio file: %s", file_path);
DLOG("Audio playback started, duration: %lu ms", duration);
DLOG("UI event triggered: %d", event_code);
DLOG("Volume changed to: %d", volume);
```

### 2. Interface Debugging

Enable interface debugging features to verify UI components:

```c
// Add interface test function
#ifdef DEBUG
static void test_ui_functionality(void) {
    // Test button response
    DLOG("Testing button functionality...");
    
    // Test font loading
    if (resource.fonts.size_16.normal) {
        DLOG("Font loaded successfully");
    } else {
        DLOG("Font loading failed");
    }
    
    // Test style application
    DLOG("UI components initialized: %d", ui_component_count);
}

// Call after initialization
test_ui_functionality();
#endif
```

### 3. Memory Analysis

Monitor memory usage to identify memory leaks:

```c
// Memory monitoring function
static void print_memory_usage(void) {
    lv_mem_monitor_t info;
    lv_mem_monitor(&info);
    printf("=== Memory Usage Report ===\n");
    printf("Total heap size: %zu bytes\n", info.total_size);
    printf("Free heap: %zu bytes\n", info.free_size);
    printf("Used heap: %zu bytes\n", info.total_size - info.free_size);
    printf("===========================\n");
}
```

## Performance Optimization

### 1. Interface Optimization

Reduce unnecessary redraws and improve rendering efficiency:

```c
// Reduce unnecessary redraws
lv_obj_add_flag(obj, LV_OBJ_FLAG_FLOATING);

// Use cache to improve performance
lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);

// Optimize image display
lv_img_set_src(img, &image_dsc);
lv_obj_set_style_img_opa(img, LV_OPA_COVER, 0);

// Reduce transparency calculations
lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
```

### 2. Memory Optimization

Optimize memory usage and reduce fragmentation:

```c
// Pre-allocate memory pool
static uint8_t audio_buffer_pool[AUDIO_BUFFER_SIZE * 4];

// Use object pool management
static album_info_t album_pool[MAX_ALBUMS];
static uint8_t album_pool_usage[MAX_ALBUMS] = {0};

// Smart memory allocation
static void* smart_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        // Trigger garbage collection
        lv_gc_collect();
        ptr = malloc(size);
    }
    return ptr;
}
```

### 3. Performance Analysis

```c
// Performance timer
static uint64_t perf_start_time;

#define PERF_START() do { \
    perf_start_time = lv_tick_get(); \
} while(0)

#define PERF_END(name) do { \
    uint64_t elapsed = lv_tick_get() - perf_start_time; \
    printf("[PERF] %s took %llu ms\n", name, elapsed); \
} while(0)

// Usage example
PERF_START();
load_audio_file(path);
PERF_END("Audio file loading");
```

## Issue Reporting

If you encounter issues not covered in this document, please submit an Issue and include the following information:

### Required Information
- **Operating System**: Ubuntu version, etc.
- **openvela Version**: Build version and date
- **Hardware Platform**: Target device or emulator
- **Reproduction Steps**: Detailed operation steps
- **Expected Behavior**: What should happen
- **Actual Behavior**: What actually happened

### Optional Information
- **Log Output**: Relevant error logs
- **Configuration Files**: Relevant configuration information
- **Environment Variables**: Possibly relevant environment settings

---

*Last updated: 2025-09-29*
