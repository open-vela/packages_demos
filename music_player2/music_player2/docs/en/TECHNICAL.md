# Vela Audio Technical Documentation

## Table of Contents

- [Technical Architecture](#technical-architecture)
- [API Reference](#api-reference)
- [Development Guide](#development-guide)

## Technical Architecture

### System Overview

Vela Audio adopts a modular layered architecture design to ensure code maintainability, scalability, and high performance. The entire system is based on an event-driven model, separating functional modules through clear interfaces.

**Core Design Principles**:
- **Modularity**: Each functional module is independently designed with clear responsibilities
- **Layered Architecture**: Clear hierarchical structure reducing coupling
- **Event-Driven**: Responsive design based on LVGL 9.x event system
- **Resource Management**: Unified resource lifecycle management
- **State Machine**: Predictable state transitions and management

### Architecture Layers

The system adopts a six-layer architecture design, from bottom to top including:

```
┌─────────────────────────────────────────┐
│           User Interface Layer (LVGL 9.x) │  ← Graphics rendering and touch interaction
├─────────────────────────────────────────┤
│             UI Module Layer             │  ← Splash screen, main interface, playlist manager
├─────────────────────────────────────────┤
│            Business Logic Layer         │  ← State management, playback control, playlist logic
├─────────────────────────────────────────┤
│             Service Layer               │  ← Audio control, file management, Wi-Fi management
├─────────────────────────────────────────┤
│        System Abstraction Layer (NuttX) │  ← Audio drivers, file system, network stack
├─────────────────────────────────────────┤
│           Hardware Abstraction Layer    │  ← Audio hardware, storage devices, network hardware
└─────────────────────────────────────────┘
```

### Data Flow Diagram

The system's data flow mainly includes 3 primary paths:

#### 1. Application Startup Flow
```
main() → splash_screen_create() → Startup animation → app_create() → Main interface
   ↓
Resource initialization → Font loading → Configuration reading → UI component creation
```

#### 2. User Interaction Flow
```
User operation → LVGL 9.x event → Event handler → State update → UI refresh
    ↓
Audio control → Playlist update → Interface feedback
```

#### 3. Module Communication Flow
```
Main interface ↔ Playlist manager ↔ Audio controller ↔ State management
   ↓              ↓              ↓
UI update      List display    Playback control
```

### Main Data Structures

#### Runtime Context
```c
struct ctx_s {
    bool resource_healthy_check;          // Resource health check flag
    album_info_t* current_album;          // Current playing album information
    
    // Playback state
    uint16_t volume;                      // Current volume (0-100)
    play_status_t play_status_prev;       // Previous playback status
    play_status_t play_status;            // Current playback status
    uint64_t current_time;                // Current playback time (milliseconds)
    
    // Timer management
    struct {
        lv_timer_t* volume_bar_countdown;      // Volume bar auto-hide countdown
        lv_timer_t* playback_progress_update;  // Playback progress update timer
        lv_timer_t* refresh_date_time;         // Date time refresh timer
    } timers;
    
    audioctl_s* audioctl;                 // Audio control handle
};
```

#### Resource Management Structure
```c
struct resource_s {
    // UI component management
    struct {
        lv_obj_t* time;                    // Time display label
        lv_obj_t* date;                    // Date display label
        lv_obj_t* player_group;            // Player control group
        lv_obj_t* album_cover_container;   // Circular cover container
        lv_obj_t* volume_bar;              // Volume bar component
        // ... more UI components
    } ui;
    
    // Font resource management
    struct {
        struct { const lv_font_t* normal; } size_16;
        struct { const lv_font_t* bold; } size_22;
        struct { const lv_font_t* normal; } size_24;
        // ... more fonts
    } fonts;
    
    // Music data
    album_info_t* albums;                  // Album information array
    uint8_t album_count;                   // Total album count
};
```

### State Machine Design

#### Playback State Definition
```c
typedef enum {
    PLAY_STATUS_STOP,    // Stop state
    PLAY_STATUS_PLAY,    // Play state
    PLAY_STATUS_PAUSE,   // Pause state
} play_status_t;
```

#### State Transition Table
```c
typedef struct {
    play_status_t from_state;     // Source state
    play_status_t to_state;       // Target state
    void (*action)(void);         // Transition action
} state_transition_t;

// State transition table
static const state_transition_t transitions[] = {
    {PLAY_STATUS_STOP,  PLAY_STATUS_PLAY,  action_start_playback},
    {PLAY_STATUS_PLAY,  PLAY_STATUS_PAUSE, action_pause_playback},
    {PLAY_STATUS_PAUSE, PLAY_STATUS_PLAY,  action_resume_playback},
    {PLAY_STATUS_PLAY,  PLAY_STATUS_STOP,  action_stop_playback},
    {PLAY_STATUS_PAUSE, PLAY_STATUS_STOP,  action_stop_playback},
};
```

## API Reference

### Core API

#### Application Control
```c
/**
 * @brief Create and initialize music player application
 */
void app_create(void);

/**
 * @brief Set playback status
 * @param status Playback status
 */
void app_set_play_status(play_status_t status);

/**
 * @brief Switch to specified album
 * @param index Album index (0-based)
 */
void app_switch_to_album(int index);
```

#### Splash Screen Module API
```c
/**
 * @brief Create splash screen
 */
void splash_screen_create(void);
```

#### Playlist Manager API
```c
/**
 * @brief Create professional playlist interface
 * @param parent Parent container object
 */
void playlist_manager_create(lv_obj_t* parent);

/**
 * @brief Refresh playlist display
 */
void playlist_manager_refresh(void);

/**
 * @brief Close playlist
 */
void playlist_manager_close(void);

/**
 * @brief Check if playlist is open
 * @return true if open, false if closed
 */
bool playlist_manager_is_open(void);
```

#### Audio Control API
```c
/**
 * @brief Initialize audio controller
 * @param file_path Audio file path
 * @return Audio control handle, NULL on failure
 */
audioctl_s* audio_ctl_init_nxaudio(const char* file_path);

/**
 * @brief Start playback
 * @param audioctl Audio control handle
 * @return 0 on success, -1 on failure
 */
int audio_ctl_start(audioctl_s* audioctl);

/**
 * @brief Pause playback
 * @param audioctl Audio control handle
 * @return 0 on success, -1 on failure
 */
int audio_ctl_pause(audioctl_s* audioctl);

/**
 * @brief Stop playback
 * @param audioctl Audio control handle
 * @return 0 on success, -1 on failure
 */
int audio_ctl_stop(audioctl_s* audioctl);

/**
 * @brief Set volume
 * @param audioctl Audio control handle
 * @param volume Volume value (0-100)
 * @return 0 on success, -1 on failure
 */
int audio_ctl_set_volume(audioctl_s* audioctl, uint16_t volume);

/**
 * @brief Seek to specified position
 * @param audioctl Audio control handle
 * @param position Position (seconds)
 * @return 0 on success, -1 on failure
 */
int audio_ctl_seek(audioctl_s* audioctl, uint32_t position);

/**
 * @brief Get current playback position
 * @param audioctl Audio control handle
 * @return Current position (seconds), -1 on failure
 */
int audio_ctl_get_position(audioctl_s* audioctl);
```

#### Font Configuration API
```c
/**
 * @brief Initialize font system
 */
void font_system_init(void);

/**
 * @brief Get font by size
 * @param size Font size
 * @return Font pointer, returns default font on failure
 */
const lv_font_t* get_font_by_size(int size);

/**
 * @brief Set UTF-8 text
 * @param label Label object
 * @param text Text content
 * @param font Font
 */
void set_label_utf8_text(lv_obj_t* label, const char* text, const lv_font_t* font);
```

### Error Codes

```c
// Success codes
#define MUSIC_ERROR_OK              0    // Operation successful

// General error codes
#define MUSIC_ERROR_INVALID_PARAM  -1    // Invalid parameter
#define MUSIC_ERROR_NO_MEMORY      -2    // Insufficient memory
#define MUSIC_ERROR_TIMEOUT        -3    // Operation timeout

// File-related error codes
#define MUSIC_ERROR_FILE_NOT_FOUND -10   // File not found
#define MUSIC_ERROR_FILE_READ      -11   // File read failure
#define MUSIC_ERROR_FILE_WRITE     -12   // File write failure

// Audio-related error codes
#define MUSIC_ERROR_AUDIO_INIT     -20   // Audio initialization failure
#define MUSIC_ERROR_AUDIO_PLAY     -21   // Audio playback failure

// Network-related error codes
#define MUSIC_ERROR_NETWORK        -30   // Network error
#define MUSIC_ERROR_WIFI_CONNECT   -31   // Wi-Fi connection failure

// UI-related error codes
#define MUSIC_ERROR_UI_INIT        -50   // UI initialization failure
#define MUSIC_ERROR_FONT_LOAD      -51   // Font loading failure
```

## Development Guide

### Code Standards

#### C Code Style
```c
// Function naming: snake_case
static void app_create_main_page(void);
static bool init_audio_system(void);

// Variable naming: snake_case
static bool resource_healthy_check = false;
static uint32_t current_playback_time = 0;

// Constant naming: UPPER_CASE
#define MAX_ALBUM_COUNT 100
#define DEFAULT_VOLUME 50

// Structure naming: snake_case_t
typedef struct album_info_s {
    const char* name;
    const char* artist;
    uint32_t duration;
} album_info_t;
```

#### Comment Standards
```c
/**
 * @brief Brief function description
 * @param param1 Parameter 1 description
 * @param param2 Parameter 2 description
 * @return Return value description
 */
int function_name(int param1, const char* param2);

// Single-line comments for simple explanations
int volume = 50; // Default volume level
```

#### Error Handling Standards
```c
// Unified error handling pattern
int audio_operation(audioctl_s* ctl) {
    // Parameter validation
    if (!ctl) {
        return MUSIC_ERROR_INVALID_PARAM;
    }
    
    // Execute operation
    int result = low_level_operation(ctl);
    if (result < 0) {
        return MUSIC_ERROR_AUDIO_PLAY;
    }
    
    return MUSIC_ERROR_OK;
}
```

### Testing Framework

#### Unit Testing
```c
// test/test_audio_ctl.c
#include "unity.h"
#include "audio_ctl.h"

void test_audio_ctl_init_valid_file(void) {
    audioctl_s* ctl = audio_ctl_init_nxaudio("test.mp3");
    TEST_ASSERT_NOT_NULL(ctl);
    audio_ctl_uninit_nxaudio(ctl);
}

void test_audio_ctl_init_invalid_file(void) {
    audioctl_s* ctl = audio_ctl_init_nxaudio("nonexistent.mp3");
    TEST_ASSERT_NULL(ctl);
}
```

### Performance Considerations

#### Memory Management
```c
// Memory pool management
static uint8_t audio_buffer_pool[AUDIO_BUFFER_SIZE * 4];

static void* allocate_audio_buffer(size_t size) {
    // Simple memory pool allocation logic
    return audio_buffer_pool;
}
```

#### UI Performance Optimization
```c
// Reduce redraw frequency
static uint32_t last_ui_update = 0;

void update_ui_if_needed(void) {
    uint32_t now = lv_tick_get();
    if (now - last_ui_update < UI_UPDATE_INTERVAL_MS) {
        return; // Skip this update
    }
    
    // Perform UI update
    app_refresh_playback_status();
    last_ui_update = now;
}
```

---

*Last updated: 2025-09-29*  
