# music_player2 Technical Documentation

## Table of Contents

- [Technical Architecture](#technical-architecture)
- [API Reference](#api-reference)
- [Development Guide](#development-guide)

## Technical Architecture

### System Overview

music_player2 adopts a modular layered architecture design to ensure code maintainability, scalability, and high performance. The entire system is based on an event-driven model, separating functional modules through clear interfaces.

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
│         User Interface Layer (LVGL 9.x) │  ← Graphics rendering and touch interaction
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
    bool resource_healthy_check;
    album_info_t* current_album;
    
    uint16_t volume;
    play_status_t play_status_prev;
    play_status_t play_status;
    uint64_t current_time;
    
    struct {
        lv_timer_t* volume_bar_countdown;
        lv_timer_t* playback_progress_update;
        lv_timer_t* refresh_date_time;
    } timers;
    
    audioctl_s* audioctl;
};
```

#### Resource Management Structure
```c
struct resource_s {
    struct {
        lv_obj_t* time;
        lv_obj_t* date;
        lv_obj_t* player_group;
        lv_obj_t* album_cover_container;
        lv_obj_t* volume_bar;
    } ui;
    
    struct {
        struct { const lv_font_t* normal; } size_16;
        struct { const lv_font_t* bold; } size_22;
        struct { const lv_font_t* normal; } size_24;
    } fonts;
    
    album_info_t* albums;
    uint8_t album_count;
};
```

### State Machine Design

#### Playback State Definition
```c
typedef enum {
    PLAY_STATUS_STOP,
    PLAY_STATUS_PLAY,
    PLAY_STATUS_PAUSE,
} play_status_t;
```

#### State Transition Logic

State transitions are implemented using `switch` statements, primarily handled in two functions:

**1. Play Button Click Event Handler** (State switching logic):
```c
play_status_t new_status;
switch (C.play_status) {
    case PLAY_STATUS_STOP:
        new_status = PLAY_STATUS_PLAY;
        break;
    case PLAY_STATUS_PLAY:
        new_status = PLAY_STATUS_PAUSE;
        break;
    case PLAY_STATUS_PAUSE:
        new_status = PLAY_STATUS_PLAY;
        break;
    default:
        return;
}
app_set_play_status(new_status);
```

**2. State Refresh Function** (State action execution):
```c
static void app_refresh_play_status(void) {
    switch (C.play_status) {
    case PLAY_STATUS_STOP:
        audio_ctl_stop(C.audioctl);
        audio_ctl_uninit_nxaudio(C.audioctl);
        C.audioctl = NULL;
        break;
    case PLAY_STATUS_PLAY:
        if (C.play_status_prev == PLAY_STATUS_PAUSE) {
            audio_ctl_resume(C.audioctl);
        } else if (C.play_status_prev == PLAY_STATUS_STOP) {
            C.audioctl = audio_ctl_init_nxaudio(audio_path);
            audio_ctl_start(C.audioctl);
        }
        break;
    case PLAY_STATUS_PAUSE:
        audio_ctl_pause(C.audioctl);
        break;
    }
}
```

**State Transition Diagram**:
```
STOP ──[Click Play]──> PLAY ──[Click Pause]──> PAUSE
  ↑                                              │
  └─────────[Playback End/Error]─────────────────┘
       PAUSE ──[Click Play]──> PLAY
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
#define MUSIC_ERROR_OK              0
#define MUSIC_ERROR_INVALID_PARAM  -1
#define MUSIC_ERROR_NO_MEMORY      -2
#define MUSIC_ERROR_TIMEOUT        -3
#define MUSIC_ERROR_FILE_NOT_FOUND -10
#define MUSIC_ERROR_FILE_READ      -11
#define MUSIC_ERROR_FILE_WRITE     -12
#define MUSIC_ERROR_AUDIO_INIT     -20
#define MUSIC_ERROR_AUDIO_PLAY     -21
#define MUSIC_ERROR_NETWORK        -30
#define MUSIC_ERROR_WIFI_CONNECT   -31
#define MUSIC_ERROR_UI_INIT        -50
#define MUSIC_ERROR_FONT_LOAD      -51
```

## Development Guide

### Code Standards

#### C Code Style
```c
static void app_create_main_page(void);
static bool init_audio_system(void);

static bool resource_healthy_check = false;
static uint32_t current_playback_time = 0;

#define MAX_ALBUM_COUNT 100
#define DEFAULT_VOLUME 50

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

int volume = 50;
```

#### Error Handling Standards
```c
int audio_operation(audioctl_s* ctl) {
    if (!ctl) {
        return MUSIC_ERROR_INVALID_PARAM;
    }
    
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
static uint8_t audio_buffer_pool[AUDIO_BUFFER_SIZE * 4];

static void* allocate_audio_buffer(size_t size) {
    return audio_buffer_pool;
}
```

#### UI Performance Optimization
```c
static uint32_t last_ui_update = 0;

void update_ui_if_needed(void) {
    uint32_t now = lv_tick_get();
    if (now - last_ui_update < UI_UPDATE_INTERVAL_MS) {
        return;
    }
    
    app_refresh_playback_status();
    last_ui_update = now;
}
```

---

*Last updated: 2025-09-29*  
