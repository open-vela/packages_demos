/**
 * Music Player - LVGL based audio player
 */

#include "music_player2.h"
#include "playlist_manager.h"
#include "font_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <netutils/cJSON.h>
#include <time.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 * MODERN UI CONSTANTS
 **********************/
#define MODERN_BACKGROUND_COLOR      lv_color_hex(0x121212)
#define MODERN_CARD_COLOR           lv_color_hex(0x1E1E1E)
#define MODERN_PRIMARY_COLOR        lv_color_hex(0x00BFFF)
#define MODERN_SECONDARY_COLOR      lv_color_hex(0xFF6B6B)
#define MODERN_TEXT_PRIMARY         lv_color_hex(0xFFFFFF)
#define MODERN_TEXT_SECONDARY       lv_color_hex(0xBBBBBB)
#define MODERN_ACCENT_COLOR         lv_color_hex(0x4ECDC4)

#define COVER_SIZE                  200
#define COVER_ROTATION_DURATION     8000  // 8 seconds per rotation for visual effect

/**********************
 *      TYPEDEFS
 **********************/

/* Progress bar state management */
typedef struct {
    bool is_seeking;                // Currently seeking
    bool was_playing;              // Was playing before seeking
    uint64_t seek_preview_time;    // Preview time
    uint32_t last_update_tick;     // Last update timestamp
    lv_anim_t smooth_anim;         // Smooth animation
    bool smooth_update_enabled;    // Enable smooth updates
    int32_t target_value;          // Target value
    int32_t current_value;         // Current value
} progress_bar_state_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/* Init functions */
static void read_configs(void);
static bool init_resource(void);
static void reload_music_config(void);
static void app_create_error_page(void);
static void app_create_main_page(void);
static void app_create_top_layer(void);

// Functions
static void app_refresh_album_info(void);
static void app_refresh_date_time(void);
static void app_refresh_play_status(void);
static void app_refresh_playback_progress(void);
static void app_refresh_playlist(void);
static void app_refresh_volume_bar(void);
static void app_refresh_volume_countdown_timer(void);

/* Additional static function declarations */
static void app_set_volume(uint16_t volume);
static void app_set_playback_time(uint32_t current_time);
static void app_start_updating_date_time(void);

/* Event handler functions */
static void app_audio_event_handler(lv_event_t* e);
static void app_play_status_event_handler(lv_event_t* e);
// Removed: now using playlist_manager system
static void app_playlist_event_handler(lv_event_t* e);
static void app_switch_album_event_handler(lv_event_t* e);
static void app_volume_bar_event_handler(lv_event_t* e);
static void app_playback_progress_bar_event_handler(lv_event_t* e);

/* Timer callback functions */
static void app_refresh_date_time_timer_cb(lv_timer_t* timer);
static void app_playback_progress_update_timer_cb(lv_timer_t* timer);
static void app_volume_bar_countdown_timer_cb(lv_timer_t* timer);

static void progress_smooth_anim_cb(void* obj, int32_t value);
static void start_smooth_progress_animation(int32_t target_value);
static void reset_progress_bar_state(void);

// Variables
struct resource_s   R;
struct ctx_s        C;
struct conf_s       CF;
static progress_bar_state_t progress_state = {
    .is_seeking = false,
    .was_playing = false,
    .seek_preview_time = 0,
    .last_update_tick = 0,
    .smooth_update_enabled = true,
    .target_value = 0,
    .current_value = 0
};

const char* WEEK_DAYS[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
const lv_style_prop_t transition_props[] = {
    LV_STYLE_OPA,
    LV_STYLE_BG_OPA,
    LV_STYLE_Y,
    LV_STYLE_HEIGHT,
    LV_STYLE_PROP_FLAG_NONE
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void app_create(void)
{
    // Init resource and context structure
    lv_memzero(&R, sizeof(R));
    lv_memzero(&C, sizeof(C));
    lv_memzero(&CF, sizeof(CF));

    // System initialization
    LV_LOG_USER("Starting music player...");

    // Initialize font system
    font_system_init();
    
    // Music player starting

    read_configs();

#if WIFI_ENABLED
    CF.wifi.conn_delay = 2000000;
    wifi_connect(&CF.wifi);
#endif

    C.resource_healthy_check = init_resource();

    if (!C.resource_healthy_check) {
        app_create_error_page();
        return;
    }

    app_create_main_page();
    app_set_play_status(PLAY_STATUS_STOP);
    app_switch_to_album(0);
    app_set_volume(30);

    app_refresh_album_info();
    app_refresh_playlist();
    app_refresh_volume_bar();
    
    LV_LOG_USER("Music Player initialized");
    
    // Initialization complete
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int32_t app_get_album_index(album_info_t* album)
{
    for (int i = 0; i < R.album_count; i++) {
        if (album == &R.albums[i]) {
            return i;
        }
    }
    return -1;
}

static void app_set_volume(uint16_t volume)
{
    C.volume = volume;
    audio_ctl_set_volume(C.audioctl, C.volume);
}

void app_set_play_status(play_status_t status)
{
    C.play_status_prev = C.play_status;
    C.play_status = status;
    app_refresh_play_status();
}

void app_switch_to_album(int index)
{
    if (R.album_count == 0 || index < 0 || index >= R.album_count || C.current_album == &R.albums[index])
        return;

    C.current_album = &R.albums[index];
    
    // Reset progress bar state to avoid confusion during track switching
    reset_progress_bar_state();
    
    app_refresh_album_info();
    app_refresh_playlist();
    app_set_playback_time(0);

    if (C.play_status == PLAY_STATUS_STOP) {
        return;
    }

    app_set_play_status(PLAY_STATUS_STOP);
    app_set_play_status(PLAY_STATUS_PLAY);
}

static void app_set_playback_time(uint32_t current_time)
{
    C.current_time = current_time;

    // Perform seek operation if audio controller exists
    if (C.audioctl) {
        audio_ctl_seek(C.audioctl, C.current_time);  // Pass milliseconds
    }
    
    // Update progress bar state
    progress_state.current_value = (int32_t)current_time;
    progress_state.target_value = (int32_t)current_time;
    
    app_refresh_playback_progress();
}

static void app_refresh_date_time(void)
{
    // Check UI components
    if (!R.ui.time || !R.ui.date) {
        LV_LOG_WARN("Time/Date UI components not initialized");
        return;
    }

    // Get current time
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    
    if (current_time == NULL) {
        // Use default time if system time unavailable
        static struct tm default_time = {0};
        default_time.tm_year = 2024 - 1900;
        default_time.tm_mon = 9 - 1;
        default_time.tm_mday = 14;
        default_time.tm_hour = 14;
        default_time.tm_min = 30;
        default_time.tm_sec = 0;
        default_time.tm_wday = 6;
        current_time = &default_time;
        
        // Simulate time progression
        static int minute_counter = 0;
        default_time.tm_min = 30 + (minute_counter++ % 60);
        if (default_time.tm_min >= 60) {
            default_time.tm_min %= 60;
            default_time.tm_hour = (default_time.tm_hour + 1) % 24;
        }
    }
    
    // Update time display
    char time_str[6];
    lv_snprintf(time_str, sizeof(time_str), "%02d:%02d", current_time->tm_hour, current_time->tm_min);
    lv_label_set_text(R.ui.time, time_str);

    // Update weekday display
    char date_str[12];
    int wday = current_time->tm_wday;
    if (wday < 0 || wday > 6) wday = 0;
    lv_snprintf(date_str, sizeof(date_str), "%s", WEEK_DAYS[wday]);
    lv_label_set_text(R.ui.date, date_str);

    // Time update completed
}

static void app_refresh_volume_bar(void)
{
    int32_t volume_bar_indic_height = C.volume;

    lv_obj_set_height(R.ui.volume_bar_indic, volume_bar_indic_height);

    lv_obj_refr_size(R.ui.volume_bar_indic);
    lv_obj_update_layout(R.ui.volume_bar_indic);

    if (C.volume > 0) {
        lv_image_set_src(R.ui.audio, R.images.audio);
    } else {
        lv_image_set_src(R.ui.audio, R.images.mute);
    }
}

static void app_refresh_album_info(void)
{
    if (C.current_album) {
        // Load album cover
        if (access(C.current_album->cover, F_OK) == 0) {
            lv_image_set_src(R.ui.album_cover, C.current_album->cover);
            LV_LOG_USER("Loading album cover: %s", C.current_album->cover);
            
            // Display image properly
            lv_image_set_scale(R.ui.album_cover, 256);
            lv_image_set_inner_align(R.ui.album_cover, LV_IMAGE_ALIGN_CENTER);
            lv_obj_set_style_bg_image_opa(R.ui.album_cover, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_image_recolor_opa(R.ui.album_cover, LV_OPA_0, LV_PART_MAIN);
            
        } else {
            lv_image_set_src(R.ui.album_cover, R.images.nocover);
            LV_LOG_WARN("Album cover file not found, using default: %s", C.current_album->cover);
        }
        
        // Update song information
        const char* display_name = (C.current_album->name && strlen(C.current_album->name) > 0) ? 
                                  C.current_album->name : "Unknown Song";
        const char* display_artist = (C.current_album->artist && strlen(C.current_album->artist) > 0) ? 
                                    C.current_album->artist : "Unknown Artist";
        
        // Use font configuration system
        set_label_utf8_text(R.ui.album_name, display_name, get_font_by_size(28));
        set_label_utf8_text(R.ui.album_artist, display_artist, get_font_by_size(22));
        
        // Album information updated
    }
}

static void app_refresh_play_status(void)
{
    if (C.timers.playback_progress_update == NULL) {
        C.timers.playback_progress_update = lv_timer_create(app_playback_progress_update_timer_cb, 1000, NULL);
    }

    switch (C.play_status) {
    case PLAY_STATUS_STOP:
        lv_image_set_src(R.ui.play_btn, R.images.play);
        lv_timer_pause(C.timers.playback_progress_update);
        if (C.audioctl) {
            audio_ctl_stop(C.audioctl);
            audio_ctl_uninit_nxaudio(C.audioctl);
            C.audioctl = NULL;
        }
        break;
    case PLAY_STATUS_PLAY:
        lv_image_set_src(R.ui.play_btn, R.images.pause);
        lv_timer_resume(C.timers.playback_progress_update);
        if (C.play_status_prev == PLAY_STATUS_PAUSE)
            audio_ctl_resume(C.audioctl);
        else if (C.play_status_prev == PLAY_STATUS_STOP) {
            if (!C.current_album || C.current_album->path[0] == '\0') {
                LV_LOG_ERROR("Current album or path is empty, cannot initialize audio");
                app_set_play_status(PLAY_STATUS_STOP);
                return;
            }
            
            // Audio file path processing
            const char* audio_path = C.current_album->path;
            
            // Use audio file path
            
            // Verify if file exists
            if (access(audio_path, R_OK) != 0) {
                LV_LOG_WARN("Main path not accessible, trying backup paths...");
                
                // Extract filename
                const char* filename = strrchr(audio_path, '/');
                if (filename) {
                    filename++; // Skip '/'
                } else {
                    filename = audio_path; // If no '/' found, entire path is filename
                }
                
                // Try backup paths
                static char backup_paths[4][512];
                snprintf(backup_paths[0], sizeof(backup_paths[0]), "/data/res/musics/%s", filename);
                snprintf(backup_paths[1], sizeof(backup_paths[1]), "res/musics/%s", filename);
                snprintf(backup_paths[2], sizeof(backup_paths[2]), "/root/vela_code/apps/packages/demos/music_player2/res/musics/%s", filename);
                snprintf(backup_paths[3], sizeof(backup_paths[3]), "./res/musics/%s", filename);
                
                audio_path = NULL; // Reset path
                for (int i = 0; i < 4; i++) {
                    // Try backup paths
                    if (access(backup_paths[i], R_OK) == 0) {
                        audio_path = backup_paths[i];
                        // Found available path
                        break;
                    }
                }
                
                if (!audio_path) {
                    LV_LOG_ERROR("Cannot find audio file: %s", C.current_album->path);
                    app_set_play_status(PLAY_STATUS_STOP);
                    return;
                }
            }
            
            // Initialize audio controller
            
            // Audio controller initialization
            int retry_count = 3;
            while (retry_count > 0 && !C.audioctl) {
                C.audioctl = audio_ctl_init_nxaudio(audio_path);
                if (!C.audioctl) {
                    retry_count--;
                    LV_LOG_WARN("Audio controller init failed, retries left: %d", retry_count);
                    if (retry_count > 0) {
                        lv_delay_ms(100);
                    }
                }
            }
            
            if (!C.audioctl) {
                LV_LOG_ERROR("Audio controller init finally failed, check audio file: %s", audio_path);
                app_set_play_status(PLAY_STATUS_STOP);
                return;
            }
            
            // Start audio playback
            int ret = audio_ctl_start(C.audioctl);
            if (ret < 0) {
                LV_LOG_ERROR("Audio playback start failed: %d", ret);
                audio_ctl_uninit_nxaudio(C.audioctl);
                C.audioctl = NULL;
                app_set_play_status(PLAY_STATUS_STOP);
                return;
            }
            
            // Audio playback started successfully
        }
        break;
    case PLAY_STATUS_PAUSE:
        lv_image_set_src(R.ui.play_btn, R.images.play);
        lv_timer_pause(C.timers.playback_progress_update);
        audio_ctl_pause(C.audioctl);
        break;
    default:
        break;
    }
}

static void app_refresh_playback_progress(void)
{
    if (!C.current_album) {
        return;
    }

    uint64_t total_time = C.current_album->total_time;

    if (C.current_time > total_time) {
        app_set_play_status(PLAY_STATUS_STOP);
        C.current_time = 0;
        return;
    }

    lv_bar_set_range(R.ui.playback_progress, 0, (int32_t)total_time);
    
    // If not in dragging state, use smooth animation to update progress bar
    if (!progress_state.is_seeking) {
        if (progress_state.smooth_update_enabled) {
            // Check if smooth animation needs to be started
            int32_t current_value = lv_bar_get_value(R.ui.playback_progress);
            int32_t new_value = (int32_t)C.current_time;
            
            // If difference is large (over 2 seconds), use smooth animation
            if (abs(new_value - current_value) > 2000) {
                start_smooth_progress_animation(new_value);
            } else {
                // Small difference, update directly
                lv_bar_set_value(R.ui.playback_progress, new_value, LV_ANIM_OFF);
                progress_state.current_value = new_value;
            }
        } else {
            lv_bar_set_value(R.ui.playback_progress, (int32_t)C.current_time, LV_ANIM_OFF);
        }
    }
    // If in dragging state, don't update progress bar display to avoid conflicts with user operations

    // Update time display (unless in drag preview)
    if (!progress_state.is_seeking) {
        char buff[16];
        
        uint32_t current_time_min = C.current_time / 60000;
        uint32_t current_time_sec = (C.current_time % 60000) / 1000;
        uint32_t total_time_min = total_time / 60000;
        uint32_t total_time_sec = (total_time % 60000) / 1000;

        lv_snprintf(buff, sizeof(buff), "%02d:%02d", current_time_min, current_time_sec);
        lv_span_set_text(R.ui.playback_current_time, buff);
        lv_snprintf(buff, sizeof(buff), "%02d:%02d", total_time_min, total_time_sec);
        lv_span_set_text(R.ui.playback_total_time, buff);
    }
}

static void app_refresh_playlist(void)
{
    // Use new playlist manager to refresh
    // If playlist manager is open, refresh its content
    if (playlist_manager_is_open()) {
        playlist_manager_refresh();
    }
    
    // Keep this function for compatibility with existing calls, but actual logic has been moved to playlist_manager
    LV_LOG_USER("Playlist refresh triggered - using new playlist manager system");
}

static void app_volume_bar_countdown_timer_cb(lv_timer_t* timer)
{
    LV_UNUSED(timer);
    lv_obj_set_state(R.ui.volume_bar, LV_STATE_DEFAULT, true);
    lv_obj_set_state(R.ui.volume_bar, LV_STATE_USER_1, false);
}

static void app_playback_progress_update_timer_cb(lv_timer_t* timer)
{
    LV_UNUSED(timer);

    // Check if audio controller is valid
    if (!C.audioctl) {
        LV_LOG_WARN("Audio controller invalid, stopping progress updates");
        return;
    }
    
    // Check playback state
    if (C.play_status != PLAY_STATUS_PLAY) {
        return;  // Don't update progress when not in playback state
    }
    
    // If user is dragging progress bar, pause automatic updates to avoid conflicts
    if (progress_state.is_seeking) {
        return;
    }

    // Get current playback position
    int position = audio_ctl_get_position(C.audioctl);
    if (position >= 0) {
        uint64_t new_time = position * 1000;
        
        // Check if time has significant changes to avoid unnecessary UI updates
        if (abs((int64_t)new_time - (int64_t)C.current_time) > 100) { // Reduce threshold to 100ms
            C.current_time = new_time;
            app_refresh_playback_progress();
        }
        
        // Output debug information every 10 seconds
        static int debug_counter = 0;
        if (++debug_counter >= 10) {
            // Playback progress update
            debug_counter = 0;
        }
    } else {
        LV_LOG_WARN("Failed to get playback position: %d", position);
    }
}

static void app_refresh_date_time_timer_cb(lv_timer_t* timer)
{
    LV_UNUSED(timer);

    app_refresh_date_time();
}

static void app_refresh_volume_countdown_timer(void)
{
    if (C.timers.volume_bar_countdown) {
        lv_timer_set_repeat_count(C.timers.volume_bar_countdown, 1);
        lv_timer_reset(C.timers.volume_bar_countdown);
        lv_timer_resume(C.timers.volume_bar_countdown);
    } else {
        C.timers.volume_bar_countdown = lv_timer_create(app_volume_bar_countdown_timer_cb, 3000, NULL);
        lv_timer_set_auto_delete(C.timers.volume_bar_countdown, false);
    }
}

static void app_playlist_event_handler(lv_event_t* e)
{
    // Static variable to prevent repeated clicks
    static uint32_t last_click_time = 0;
    uint32_t current_time = lv_tick_get();
    
    // Playlist event handling
    if (!e) {
        LV_LOG_WARN("Playlist button event pointer is null");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    
    // Prevent fast repeated clicks
    if (code == LV_EVENT_CLICKED) {
        if (current_time - last_click_time < 500) {
            LV_LOG_WARN("Click too fast, ignoring repeat click");
            return;
        }
        last_click_time = current_time;
    }
    
    // Support multiple interaction modes
    if (code != LV_EVENT_CLICKED && code != LV_EVENT_PRESSED) {
        return;  // Handle click and press events
    }
    
    // Provide user feedback
    if (code == LV_EVENT_PRESSED) {
        // Visual feedback when pressed
        lv_obj_t* target = lv_event_get_target(e);
        if (target) {
            lv_obj_set_style_transform_scale(target, 240, LV_PART_MAIN | LV_STATE_PRESSED);  // Reduce scaling
        }
        return;
    }
    
    // Playlist button clicked
    
    // Check playlist data
    if (!R.albums || R.album_count == 0) {
        LV_LOG_WARN("Playlist is empty or not initialized, cannot display");
        
        // Error message
        if (R.ui.album_name) {
            lv_label_set_text(R.ui.album_name, "No music files found");
        }
        
        // Create message dialog
        lv_obj_t* mbox = lv_msgbox_create(lv_screen_active());
        lv_msgbox_add_title(mbox, "Empty Playlist");
        lv_msgbox_add_text(mbox, "No music files found.\nPlease add music files to continue.");
        lv_msgbox_add_close_button(mbox);
        
        return;
    }

    // Use traditional playlist - optimize memory check
    if (playlist_manager_is_open()) {
        // If playlist is already open, close it
        LV_LOG_USER("Closing playlist (songs: %d)", R.album_count);
        playlist_manager_close();
    } else {
        // Create playlist - relax memory limits
        LV_LOG_USER("Opening playlist (songs: %d)", R.album_count);
        
        // Smart container selection logic
        lv_obj_t* parent_container = lv_layer_top();
        if (!parent_container) {
            LV_LOG_WARN("Top container unavailable, using active screen");
            parent_container = lv_screen_active();
        }
        
        if (parent_container) {
            // Memory check
            lv_mem_monitor_t mem_info;
            lv_mem_monitor(&mem_info);
            // Memory status check
            
            // Try to create even with low memory, let playlist handle internally
            playlist_manager_create(parent_container);
            
            lv_mem_monitor(&mem_info);
            // Playlist creation completed
        } else {
            LV_LOG_ERROR("Cannot find suitable parent container");
        }
    }
}

static void app_volume_bar_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (!(code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING || code == LV_EVENT_PRESS_LOST)) {
        return;
    }

    lv_point_t point;
    lv_indev_t* indev = lv_indev_active();
    lv_indev_get_vect(indev, &point);

    int32_t volume_bar_height = lv_obj_get_height(R.ui.volume_bar);
    int32_t volume_bar_indic_height = lv_obj_get_height(R.ui.volume_bar_indic);

    if (volume_bar_indic_height < 0) {
        volume_bar_indic_height = 0;
    } else if (volume_bar_indic_height > volume_bar_height) {
        volume_bar_indic_height = volume_bar_height;
    }

    int32_t volume = volume_bar_indic_height - point.y;
    if (volume < 0)
        volume = 0;

    app_set_volume(volume);

    app_refresh_volume_bar();
    app_refresh_volume_countdown_timer();
}

static void app_audio_event_handler(lv_event_t* e)
{
    // Volume control event handling
    if (!e) {
        LV_LOG_ERROR("Volume button event pointer is null");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* target = lv_event_get_target(e);
    
    // Support multiple interaction modes
    if (code == LV_EVENT_PRESSED && target) {
        // Visual feedback when pressed
        lv_obj_set_style_transform_scale(target, 240, LV_PART_MAIN | LV_STATE_PRESSED);  // Reduce scaling
        return;
    }
    
    if (code == LV_EVENT_RELEASED && target) {
        // Restore scaling when released
        lv_obj_set_style_transform_scale(target, 256, LV_PART_MAIN);  // Restore normal size
        return;
    }
    
    if (code != LV_EVENT_CLICKED) {
        return;  // Only handle click events
    }
    
    // Volume button clicked

    // Volume bar status check
    if (!R.ui.volume_bar) {
        LV_LOG_ERROR("Volume bar component not initialized");
        
        // Try to reinitialize volume bar
        if (R.ui.audio) {
            LV_LOG_WARN("Trying to reinitialize volume controls...");
            // Logic to reinitialize volume bar can be added here
        }
        return;
    }

    // Smart volume bar display toggle
    bool volume_visible = lv_obj_has_state(R.ui.volume_bar, LV_STATE_USER_1);
    
    if (volume_visible) {
        LV_LOG_USER("Hiding volume bar (current volume: %d)", C.volume);
        lv_obj_set_state(R.ui.volume_bar, LV_STATE_DEFAULT, true);
        lv_obj_set_state(R.ui.volume_bar, LV_STATE_USER_1, false);
        
        // Stop countdown timer
        if (C.timers.volume_bar_countdown) {
            lv_timer_pause(C.timers.volume_bar_countdown);
        }
    } else {
        // Show volume bar
        lv_obj_set_state(R.ui.volume_bar, LV_STATE_DEFAULT, false);
        lv_obj_set_state(R.ui.volume_bar, LV_STATE_USER_1, true);
        app_refresh_volume_countdown_timer();
    }
    
    // Update volume icon status
    if (R.ui.audio) {
        if (C.volume > 0) {
            lv_image_set_src(R.ui.audio, R.images.audio);
        } else {
            lv_image_set_src(R.ui.audio, R.images.mute);
        }
    }
    
    // Memory status check
    // Memory monitoring - silent mode
    lv_mem_monitor_t mem_info;
    lv_mem_monitor(&mem_info);
    
}


static void app_switch_album_event_handler(lv_event_t* e)
{
    if (!e) {
        // Event pointer is null
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* target = lv_event_get_target(e);
    
    // Handle visual feedback for press and release
    if (code == LV_EVENT_PRESSED && target) {
        // Visual feedback when pressed
        lv_obj_set_style_transform_scale(target, 245, LV_PART_MAIN);  // Slight shrink
        lv_obj_set_style_shadow_width(target, 20, LV_PART_MAIN);
        return;
    } else if (code == LV_EVENT_RELEASED && target) {
        // Restore state when released
        lv_obj_set_style_transform_scale(target, 256, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(target, 15, LV_PART_MAIN);
        return;
    }
    
    if (code != LV_EVENT_CLICKED && code != LV_EVENT_LONG_PRESSED_REPEAT) {
        // Only handle click and long press repeat events
        return;
    }
    
    // Basic state validation
    if (R.album_count == 0) {
        LV_LOG_WARN("Playlist is empty, cannot switch songs");
        return;
    }
    
    if (!C.current_album) {
        LV_LOG_WARN("Current album is empty, trying to select first song");
        app_switch_to_album(0);
        return;
    }
    
    switch_album_mode_t direction = (switch_album_mode_t)(lv_uintptr_t)lv_event_get_user_data(e);
    
    // Validate direction parameter
    if (direction != SWITCH_ALBUM_MODE_PREV && direction != SWITCH_ALBUM_MODE_NEXT) {
        LV_LOG_ERROR("Invalid switch direction: %d", direction);
        return;
    }
    
    // Song switch operation
    
    int32_t album_index = app_get_album_index(C.current_album);
    if (album_index < 0) {
        // Cannot get current song index, reset to first song
        app_switch_to_album(0);
        return;
    }

    // Calculate new index
    int32_t new_index = album_index;
    switch (direction) {
    case SWITCH_ALBUM_MODE_PREV:
        new_index = (album_index - 1 + R.album_count) % R.album_count;
        break;
    case SWITCH_ALBUM_MODE_NEXT:
        new_index = (album_index + 1) % R.album_count;
        break;
    }
    
    // Final validation
    if (new_index < 0 || new_index >= R.album_count) {
        LV_LOG_ERROR("Calculated index is invalid: %ld (range: 0-%d)", (long)new_index, R.album_count - 1);
        return;
    }

    // Switch song

    // Execute switch
    app_switch_to_album(new_index);
    // Song switch successful
}

static void app_play_status_event_handler(lv_event_t* e)
{
    // Play button event handling
    if (!e) {
        LV_LOG_ERROR("Event pointer is null, play button event handling failed");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* target = lv_event_get_target(e);
    
    if (!target) {
        LV_LOG_ERROR("Target object is null, play button event handling failed");
        return;
    }
    
    // Handle visual feedback for press and release
    if (code == LV_EVENT_PRESSED) {
        // Visual feedback when pressed
        lv_obj_add_state(target, LV_STATE_PRESSED);
        lv_obj_set_style_transform_scale(target, 245, LV_PART_MAIN);  // Slight shrink
        lv_obj_set_style_shadow_width(target, 35, LV_PART_MAIN);
        return;
    } else if (code == LV_EVENT_RELEASED) {
        // Restore state when released
        lv_obj_clear_state(target, LV_STATE_PRESSED);
        lv_obj_set_style_transform_scale(target, 256, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(target, 25, LV_PART_MAIN);
        return;
    } else if (code != LV_EVENT_CLICKED) {
        // Only handle click, press and release events
        return;
    }
    
    // Status check - ensure player is in valid state
    if (!C.current_album && R.album_count > 0) {
        LV_LOG_WARN("No album selected currently, automatically selecting first song");
        app_switch_to_album(0);
        return;
    } else if (R.album_count == 0) {
        LV_LOG_ERROR("Playlist is empty, cannot play");
        return;
    }
    
    // Play button click handling

    // State machine handling
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
        LV_LOG_ERROR("Unknown play status: %d, operation rejected", C.play_status);
        return;
    }
    
    // State switch
    
    // Execute state switch
    app_set_play_status(new_status);
    // Play status switch completed
}

// Smooth animation callback function
static void progress_smooth_anim_cb(void* obj, int32_t value)
{
    lv_obj_t* progress_bar = (lv_obj_t*)obj;
    if (progress_bar && lv_obj_is_valid(progress_bar)) {
        progress_state.current_value = value;
        lv_bar_set_value(progress_bar, value, LV_ANIM_OFF);
    }
}

// Start smooth animation
static void start_smooth_progress_animation(int32_t target_value)
{
    if (!R.ui.playback_progress || !progress_state.smooth_update_enabled) {
        return;
    }
    
    // If currently dragging, don't start smooth animation
    if (progress_state.is_seeking) {
        return;
    }
    
    progress_state.target_value = target_value;
    
    // Stop previous animation
    lv_anim_delete(R.ui.playback_progress, progress_smooth_anim_cb);
    
    // Create new smooth animation
    lv_anim_init(&progress_state.smooth_anim);
    lv_anim_set_var(&progress_state.smooth_anim, R.ui.playback_progress);
    lv_anim_set_exec_cb(&progress_state.smooth_anim, progress_smooth_anim_cb);
    lv_anim_set_values(&progress_state.smooth_anim, progress_state.current_value, target_value);
    lv_anim_set_duration(&progress_state.smooth_anim, 200); // 200ms smooth transition
    lv_anim_set_path_cb(&progress_state.smooth_anim, lv_anim_path_ease_out);
    lv_anim_start(&progress_state.smooth_anim);
}


// Reset progress bar state
static void reset_progress_bar_state(void)
{
    progress_state.is_seeking = false;
    progress_state.was_playing = false;
    progress_state.seek_preview_time = 0;
    progress_state.last_update_tick = 0;
    progress_state.target_value = 0;
    progress_state.current_value = 0;
    
    // Stop all animations
    if (R.ui.playback_progress) {
        lv_anim_delete(R.ui.playback_progress, progress_smooth_anim_cb);
    }
    
    LV_LOG_USER("Progress bar state has been reset");
}



static void app_playback_progress_bar_event_handler(lv_event_t* e)
{
    // Progress bar interaction handling
    if (!e) {
        LV_LOG_ERROR("Progress bar event pointer is null");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    
    if (!C.current_album) {
        LV_LOG_ERROR("Current album is empty, cannot operate progress bar");
        return;
    }
    
    uint32_t current_tick = lv_tick_get();
    
    switch (code) {
    case LV_EVENT_PRESSED: {
        // Start dragging mode
        progress_state.is_seeking = true;
        progress_state.was_playing = (C.play_status == PLAY_STATUS_PLAY);
        progress_state.last_update_tick = current_tick;
        
        // Stop smooth animation and update timer
        lv_anim_delete(R.ui.playback_progress, progress_smooth_anim_cb);
        if (C.timers.playback_progress_update) {
            lv_timer_pause(C.timers.playback_progress_update);
        }
        
        // Drag state visual feedback - blue highlight and smooth animation
        lv_obj_set_height(R.ui.playback_progress, 14);  // More noticeable height change
        lv_obj_set_style_bg_color(R.ui.playback_progress, lv_color_hex(0x0078D4), LV_PART_INDICATOR);  // Microsoft blue
        lv_obj_set_style_shadow_width(R.ui.playback_progress, 12, LV_PART_INDICATOR);
        lv_obj_set_style_shadow_color(R.ui.playback_progress, lv_color_hex(0x0078D4), LV_PART_INDICATOR);
        lv_obj_set_style_shadow_opa(R.ui.playback_progress, LV_OPA_70, LV_PART_INDICATOR);
        lv_obj_set_style_radius(R.ui.playback_progress, 7, LV_PART_INDICATOR);
        lv_obj_set_style_transform_zoom(R.ui.playback_progress, 256 + 20, LV_PART_INDICATOR);  // Slight zoom effect
        
        // Drag feedback
        // Start dragging mode
        break;
    }
    case LV_EVENT_PRESSING: {
        // Real-time preview update during dragging
        if (!progress_state.is_seeking) break;
        
        // Optimize responsiveness: reduce throttling delay
        if (current_tick - progress_state.last_update_tick < 8) {  // Faster response
            break;
        }
        progress_state.last_update_tick = current_tick;
        
        lv_point_t point;
        lv_indev_t* indev = lv_indev_active();
        lv_indev_get_point(indev, &point);

        // Get progress bar coordinates and dimensions
        lv_area_t area;
        lv_obj_get_coords(R.ui.playback_progress, &area);
        
        int32_t bar_width = lv_area_get_width(&area);
        int32_t relative_x = point.x - area.x1;
        
        // Simplify boundary check and expand touch area
        const int32_t touch_tolerance = 30;  // Expand touch tolerance
        if (relative_x < -touch_tolerance) relative_x = 0;
        else if (relative_x > bar_width + touch_tolerance) relative_x = bar_width;
        else if (relative_x < 0) relative_x = 0;
        else if (relative_x > bar_width) relative_x = bar_width;
        
        // Calculate new preview time
        uint64_t total_time = C.current_album->total_time;
        uint64_t new_time = (uint64_t)relative_x * total_time / bar_width;
        
        // Time range check
        if (new_time > total_time) new_time = total_time;
        
        progress_state.seek_preview_time = new_time;
        
        // Drag position calculation completed
        
        // Update UI display
        progress_state.current_value = (int32_t)new_time;
        lv_bar_set_value(R.ui.playback_progress, (int32_t)new_time, LV_ANIM_OFF);
        
        // Update time display
        char buff[16];
        uint32_t preview_min = new_time / 60000;
        uint32_t preview_sec = (new_time % 60000) / 1000;
        lv_snprintf(buff, sizeof(buff), "%02d:%02d", preview_min, preview_sec);
        lv_span_set_text(R.ui.playback_current_time, buff);
        
        // Preview feedback - silent mode
        
        break;
    }
    case LV_EVENT_RELEASED: {
        // Execute seek operation on release
        if (!progress_state.is_seeking) break;
        
        progress_state.is_seeking = false;
        
        // Restore progress bar normal style - smooth transition
        lv_obj_set_height(R.ui.playback_progress, 8);
        lv_obj_set_style_bg_color(R.ui.playback_progress, lv_color_hex(0xFF4757), LV_PART_INDICATOR);
        lv_obj_set_style_shadow_width(R.ui.playback_progress, 0, LV_PART_INDICATOR);
        lv_obj_set_style_radius(R.ui.playback_progress, 4, LV_PART_INDICATOR);
        lv_obj_set_style_transform_zoom(R.ui.playback_progress, 256, LV_PART_INDICATOR);  // Restore normal size
        
        // Execute seek operation
        uint64_t seek_time = progress_state.seek_preview_time;
        
        // Safe boundary check
        if (seek_time > C.current_album->total_time) {
            seek_time = C.current_album->total_time;
        }
        
        if (C.audioctl && seek_time <= C.current_album->total_time) {
            // Execute seek, pass milliseconds
            int seek_result = audio_ctl_seek(C.audioctl, seek_time);
            
            if (seek_result == 0) {
                // Seek successful, update current time
                C.current_time = seek_time;
                progress_state.current_value = (int32_t)seek_time;
                progress_state.target_value = (int32_t)seek_time;
                
                // Force refresh progress display
                app_refresh_playback_progress();
            } else {
                // Seek failed, restore to original state
                C.current_time = progress_state.current_value;
                app_refresh_playback_progress();
            }
        } else {
            // Audio controller invalid or time out of range, restore state
            app_refresh_playback_progress();
        }
        
        // Smart restore play state
        if (progress_state.was_playing) {
            if (C.play_status == PLAY_STATUS_PAUSE) {
                // If currently paused and was playing before, resume playback
                if (C.audioctl) {
                    audio_ctl_resume(C.audioctl);
                    app_set_play_status(PLAY_STATUS_PLAY);
                }
            }
        }
        
        // Restore timers and smooth updates
        if (C.timers.playback_progress_update) {
            lv_timer_resume(C.timers.playback_progress_update);
        }
        progress_state.smooth_update_enabled = true;
        
        // Drag operation completed
        break;
    }
    case LV_EVENT_CLICKED: {
        // Single click quick jump
        if (progress_state.is_seeking) break;
        
        lv_point_t point;
        lv_indev_t* indev = lv_indev_active();
        lv_indev_get_point(indev, &point);
        
        // Get progress bar coordinates and dimensions
        lv_area_t area;
        lv_obj_get_coords(R.ui.playback_progress, &area);
        
        int32_t bar_width = lv_area_get_width(&area);
        int32_t relative_x = point.x - area.x1;
        
        // Boundary check
        if (relative_x < 0) relative_x = 0;
        if (relative_x > bar_width) relative_x = bar_width;
        
        // Calculate new playback time
        uint64_t total_time = C.current_album->total_time;
        uint64_t new_time = (uint64_t)relative_x * total_time / bar_width;
        
        if (new_time > total_time) new_time = total_time;
        
        // Click jump operation
        
        // Immediately update progress bar value for instant feedback
        lv_bar_set_value(R.ui.playback_progress, (int32_t)new_time, LV_ANIM_ON);
        
        // Execute actual jump
        if (C.audioctl) {
            int seek_result = audio_ctl_seek(C.audioctl, new_time);
            
            if (seek_result == 0) {
                C.current_time = new_time;
                
                // Update time display
                char buff[16];
                uint32_t current_min = new_time / 60000;
                uint32_t current_sec = (new_time % 60000) / 1000;
                lv_snprintf(buff, sizeof(buff), "%02d:%02d", current_min, current_sec);
                lv_span_set_text(R.ui.playback_current_time, buff);
            } else {
                // Seek failed, restore original progress
                app_refresh_playback_progress();
            }
        }
        break;
    }
    case LV_EVENT_PRESS_LOST: {
        // Safely restore state when focus is unexpectedly lost
        if (progress_state.is_seeking) {
            progress_state.is_seeking = false;
            
            // Restore normal style
            lv_obj_set_height(R.ui.playback_progress, 8);
            lv_obj_set_style_bg_color(R.ui.playback_progress, lv_color_hex(0xFF4757), LV_PART_INDICATOR);
            lv_obj_set_style_shadow_width(R.ui.playback_progress, 0, LV_PART_INDICATOR);
            lv_obj_set_style_radius(R.ui.playback_progress, 4, LV_PART_INDICATOR);
            lv_obj_set_style_transform_zoom(R.ui.playback_progress, 256, LV_PART_INDICATOR);  // Restore normal size
            
            // Restore timer
            if (C.timers.playback_progress_update) {
                lv_timer_resume(C.timers.playback_progress_update);
            }
            
            // Restore correct progress display
            app_refresh_playback_progress();
            
            // Drag unexpectedly interrupted, restored
        }
        break;
    }
    case LV_EVENT_LONG_PRESSED: {
        // Long press to enter precise adjustment mode
        if (!progress_state.is_seeking) {
            // Start precise adjustment mode
            progress_state.is_seeking = true;
            progress_state.was_playing = (C.play_status == PLAY_STATUS_PLAY);
            progress_state.last_update_tick = current_tick;
            
            // Stop timers and animations
            lv_anim_delete(R.ui.playback_progress, progress_smooth_anim_cb);
            if (C.timers.playback_progress_update) {
                lv_timer_pause(C.timers.playback_progress_update);
            }
        }
        
        // Precise adjustment mode visual feedback - green highlight
        lv_obj_set_height(R.ui.playback_progress, 16);  // Maximum height
        lv_obj_set_style_bg_color(R.ui.playback_progress, lv_color_hex(0x00C851), LV_PART_INDICATOR);  // Green
        lv_obj_set_style_shadow_width(R.ui.playback_progress, 8, LV_PART_INDICATOR);
        lv_obj_set_style_shadow_color(R.ui.playback_progress, lv_color_hex(0x00C851), LV_PART_INDICATOR);
        lv_obj_set_style_shadow_opa(R.ui.playback_progress, LV_OPA_80, LV_PART_INDICATOR);
        lv_obj_set_style_radius(R.ui.playback_progress, 8, LV_PART_INDICATOR);
        lv_obj_set_style_transform_zoom(R.ui.playback_progress, 256 + 30, LV_PART_INDICATOR);  // More noticeable zoom
        
        // Precise adjustment mode activated
        break;
    }
    default:
        break;
    }
}

static bool init_resource(void)
{
    // Simplified version: only initialize basic functional modules
    
    // Use LVGL built-in fonts - only use fonts enabled in configuration
    R.fonts.size_16.normal = &lv_font_montserrat_16;
    R.fonts.size_22.bold = &lv_font_montserrat_22;
    R.fonts.size_24.normal = &lv_font_montserrat_24;
    R.fonts.size_28.normal = &lv_font_montserrat_32;  // Use 32pt font instead of 28pt
    R.fonts.size_60.bold = &lv_font_montserrat_32;    // Use 32pt font instead of 60pt

    // Font check - built-in fonts are always available
    if (R.fonts.size_16.normal == NULL ||
        R.fonts.size_22.bold == NULL   ||
        R.fonts.size_24.normal == NULL ||
        R.fonts.size_28.normal == NULL ||
        R.fonts.size_60.bold == NULL ) {
        return false;
    }

    // Modern UI Styles
    lv_style_init(&R.styles.button_default);
    lv_style_init(&R.styles.button_pressed);
    lv_style_init(&R.styles.circular_cover);
    lv_style_init(&R.styles.vinyl_ring);
    lv_style_init(&R.styles.vinyl_center);
    lv_style_init(&R.styles.gradient_progress);
    lv_style_init(&R.styles.frosted_glass);
    lv_style_init(&R.styles.modern_card);

    // Professional button styles - touch-friendly design
    lv_style_set_opa(&R.styles.button_default, LV_OPA_COVER);
    lv_style_set_opa(&R.styles.button_pressed, LV_OPA_80);  // Slightly reduce opacity when clicked
    
    // Enhanced button click feedback
    lv_style_set_shadow_width(&R.styles.button_default, 8);
    lv_style_set_shadow_color(&R.styles.button_default, MODERN_PRIMARY_COLOR);
    lv_style_set_shadow_opa(&R.styles.button_default, LV_OPA_40);
    
    // Shadow effect when pressed
    lv_style_set_shadow_width(&R.styles.button_pressed, 15);
    lv_style_set_shadow_color(&R.styles.button_pressed, MODERN_PRIMARY_COLOR);
    lv_style_set_shadow_opa(&R.styles.button_pressed, LV_OPA_60);
    
    // Add button borders to improve visibility
    lv_style_set_border_width(&R.styles.button_default, 2);
    lv_style_set_border_color(&R.styles.button_default, lv_color_hex(0x4B5563));
    lv_style_set_border_opa(&R.styles.button_default, LV_OPA_50);
    
    // Border brightens when pressed
    lv_style_set_border_width(&R.styles.button_pressed, 2);
    lv_style_set_border_color(&R.styles.button_pressed, MODERN_PRIMARY_COLOR);
    lv_style_set_border_opa(&R.styles.button_pressed, LV_OPA_80);

    // Circular cover style - enhanced circular border effect
    lv_style_set_radius(&R.styles.circular_cover, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&R.styles.circular_cover, 6);  // Increase border width to make it more visible
    lv_style_set_border_color(&R.styles.circular_cover, lv_color_hex(0x3B82F6));  // Blue border more prominent
    lv_style_set_border_opa(&R.styles.circular_cover, LV_OPA_80);  // Slightly transparent
    lv_style_set_shadow_width(&R.styles.circular_cover, 25);  // Enhanced shadow
    lv_style_set_shadow_color(&R.styles.circular_cover, lv_color_hex(0x3B82F6));  // Blue shadow
    lv_style_set_shadow_opa(&R.styles.circular_cover, LV_OPA_30);
    lv_style_set_shadow_spread(&R.styles.circular_cover, 5);

    // Vinyl ring style - outer ring style
    lv_style_set_radius(&R.styles.vinyl_ring, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&R.styles.vinyl_ring, 8);  // Thick border simulating vinyl edge
    lv_style_set_border_color(&R.styles.vinyl_ring, lv_color_hex(0x1A1A1A));  // Deep black
    lv_style_set_border_opa(&R.styles.vinyl_ring, LV_OPA_COVER);
    lv_style_set_bg_color(&R.styles.vinyl_ring, lv_color_hex(0x0F0F0F));  // Very deep background
    lv_style_set_bg_opa(&R.styles.vinyl_ring, LV_OPA_30);  // Semi-transparent
    lv_style_set_shadow_width(&R.styles.vinyl_ring, 30);
    lv_style_set_shadow_color(&R.styles.vinyl_ring, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&R.styles.vinyl_ring, LV_OPA_70);
    lv_style_set_shadow_spread(&R.styles.vinyl_ring, 8);

    // Vinyl center style - center hole style
    lv_style_set_radius(&R.styles.vinyl_center, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&R.styles.vinyl_center, lv_color_hex(0x1A1A1A));  // Deep black center
    lv_style_set_bg_opa(&R.styles.vinyl_center, LV_OPA_COVER);
    lv_style_set_border_width(&R.styles.vinyl_center, 2);
    lv_style_set_border_color(&R.styles.vinyl_center, lv_color_hex(0x333333));
    lv_style_set_border_opa(&R.styles.vinyl_center, LV_OPA_COVER);

    // Gradient progress bar style
    lv_style_set_bg_color(&R.styles.gradient_progress, MODERN_PRIMARY_COLOR);
    lv_style_set_bg_grad_color(&R.styles.gradient_progress, MODERN_SECONDARY_COLOR);
    lv_style_set_bg_grad_dir(&R.styles.gradient_progress, LV_GRAD_DIR_HOR);
    lv_style_set_radius(&R.styles.gradient_progress, 10);

    // Frosted glass style (simplified without backdrop filter)
    lv_style_set_bg_color(&R.styles.frosted_glass, lv_color_hex(0x1E1E1E));
    lv_style_set_bg_opa(&R.styles.frosted_glass, LV_OPA_70);
    lv_style_set_radius(&R.styles.frosted_glass, 20);

    // Modern card style
    lv_style_set_bg_color(&R.styles.modern_card, MODERN_CARD_COLOR);
    lv_style_set_bg_opa(&R.styles.modern_card, LV_OPA_90);
    lv_style_set_radius(&R.styles.modern_card, 25);
    lv_style_set_shadow_width(&R.styles.modern_card, 15);
    lv_style_set_shadow_color(&R.styles.modern_card, lv_color_black());
    lv_style_set_shadow_opa(&R.styles.modern_card, LV_OPA_30);

    // transition animations
    lv_style_transition_dsc_init(&R.styles.transition_dsc, transition_props, &lv_anim_path_ease_in_out, 300, 0, NULL);
    lv_style_transition_dsc_init(&R.styles.button_transition_dsc, transition_props, &lv_anim_path_ease_in_out, 150, 0, NULL);
    lv_style_set_transition(&R.styles.button_default, &R.styles.button_transition_dsc);
    lv_style_set_transition(&R.styles.button_pressed, &R.styles.button_transition_dsc);

    // images
    R.images.playlist = ICONS_ROOT "/playlist.png";
    R.images.previous = ICONS_ROOT "/previous.png";
    R.images.play = ICONS_ROOT "/play.png";
    R.images.pause = ICONS_ROOT "/pause.png";
    R.images.next = ICONS_ROOT "/next.png";
    R.images.audio = ICONS_ROOT "/audio.png";
    R.images.mute = ICONS_ROOT "/mute.png";
    R.images.music = ICONS_ROOT "/music.png";
    R.images.nocover = ICONS_ROOT "/nocover.png";
    
    // Background image disabled - user requested to remove background.png
    // R.images.background = ICONS_ROOT "/background.png";  // Removed
    R.images.background = NULL;  // Explicitly set to NULL

    // albums
    reload_music_config();

    return true;
}

static void app_create_top_layer(void)
{
    lv_obj_t* top_layer = lv_layer_top();
    lv_obj_set_scroll_dir(top_layer, LV_DIR_NONE);
    lv_obj_set_style_bg_color(top_layer, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(top_layer, LV_OPA_COVER, LV_STATE_USER_1);
    lv_obj_set_style_bg_opa(top_layer, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_transition(top_layer, &R.styles.transition_dsc, LV_STATE_DEFAULT);
    lv_obj_set_style_transition(top_layer, &R.styles.transition_dsc, LV_STATE_USER_1);
    
    // Note: Only create volume bar, playlist has been moved to new playlist_manager system

    // VOLUME BAR
    R.ui.volume_bar = lv_obj_create(top_layer);
    lv_obj_remove_style_all(R.ui.volume_bar);
    lv_obj_set_size(R.ui.volume_bar, 60, 180);
    lv_obj_set_style_bg_color(R.ui.volume_bar, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(R.ui.volume_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_opa(R.ui.volume_bar, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_opa(R.ui.volume_bar, LV_OPA_COVER, LV_STATE_USER_1);
    lv_obj_set_style_border_width(R.ui.volume_bar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(R.ui.volume_bar, 16, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(R.ui.volume_bar, true, LV_PART_MAIN);
    lv_obj_align(R.ui.volume_bar, LV_ALIGN_BOTTOM_RIGHT, -45, -95);
    lv_obj_set_style_transition(R.ui.volume_bar, &R.styles.transition_dsc, LV_STATE_DEFAULT);

    R.ui.volume_bar_indic = lv_obj_create(R.ui.volume_bar);
    lv_obj_remove_style_all(R.ui.volume_bar_indic);
    lv_obj_set_style_bg_color(R.ui.volume_bar_indic, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(R.ui.volume_bar_indic, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_size(R.ui.volume_bar_indic, LV_PCT(100), 40);
    lv_obj_align(R.ui.volume_bar_indic, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Old playlist system removed - now using new system in playlist_manager.c
    // Set playlist-related UI pointers to NULL to avoid misuse elsewhere
    R.ui.playlist_base = NULL;
    R.ui.playlist = NULL;

    lv_obj_add_flag(R.ui.volume_bar_indic, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(R.ui.volume_bar, app_volume_bar_event_handler, LV_EVENT_ALL, NULL);
}

static void app_create_error_page(void)
{
    lv_obj_t* root = lv_screen_active();
    lv_obj_t* label = lv_label_create(root);
    lv_obj_set_width(label, LV_PCT(80));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label, "Vela Music Player\nResource loading failed\nPlease check device and configuration");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFF6B6B), LV_PART_MAIN);
    lv_obj_center(label);
}

static void app_create_main_page(void)
{
    lv_obj_t* root = lv_screen_active();

    // Professional solid background design - remove background.png dependency
    // Adopt dark professional theme to enhance user experience and performance
    lv_obj_set_style_bg_color(root, lv_color_hex(0x121212), LV_PART_MAIN);  // Dark theme
    lv_obj_set_style_bg_grad_color(root, lv_color_hex(0x0F0F0F), LV_PART_MAIN);  // Deeper gradient
        lv_obj_set_style_bg_grad_dir(root, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, LV_PART_MAIN);
    
    // Dark background applied
    lv_obj_set_style_border_width(root, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(root, 16, LV_PART_MAIN);

    // Top status bar - WiFi signal, battery, time area
    lv_obj_t* status_bar = lv_obj_create(root);
    lv_obj_remove_style_all(status_bar);
    lv_obj_set_size(status_bar, LV_PCT(100), 48);
    lv_obj_add_style(status_bar, &R.styles.frosted_glass, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(status_bar, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(status_bar, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(status_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Left side car brand logo
    lv_obj_t* brand_label = lv_label_create(status_bar);
    lv_label_set_text(brand_label, "Vela Audio");
    lv_obj_set_style_text_font(brand_label, R.fonts.size_22.bold, LV_PART_MAIN);
    lv_obj_set_style_text_color(brand_label, lv_color_hex(0x3B82F6), LV_PART_MAIN); // Neon blue

    // Right side status info (signal, battery, time)
    lv_obj_t* status_info = lv_obj_create(status_bar);
    lv_obj_remove_style_all(status_info);
    lv_obj_set_size(status_info, LV_PCT(60), LV_SIZE_CONTENT);  // Give status info more space
    lv_obj_set_flex_flow(status_info, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_info, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // WiFi signal
    lv_obj_t* wifi_label = lv_label_create(status_info);
    lv_label_set_text(wifi_label, "WiFi");
    lv_obj_set_style_text_font(wifi_label, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(wifi_label, lv_color_hex(0xBBBBBB), LV_PART_MAIN);
    lv_obj_set_style_margin_right(wifi_label, 16, LV_PART_MAIN);

    // Battery
    lv_obj_t* battery_label = lv_label_create(status_info);
    lv_label_set_text(battery_label, "85%");
    lv_obj_set_style_text_font(battery_label, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(battery_label, lv_color_hex(0xBBBBBB), LV_PART_MAIN);
    lv_obj_set_style_margin_right(battery_label, 20, LV_PART_MAIN);

    // Time and date container - vertical layout to save horizontal space
    lv_obj_t* time_container = lv_obj_create(status_info);
    lv_obj_remove_style_all(time_container);
    lv_obj_set_size(time_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(time_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(time_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(time_container, 0, LV_PART_MAIN);
    
    // Time display - larger font
    lv_obj_t* time_label = lv_label_create(time_container);
    R.ui.time = time_label;
    lv_label_set_text(time_label, "14:28");  // Use example time to ensure sufficient width
    lv_obj_set_style_text_font(time_label, R.fonts.size_24.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_set_style_margin_bottom(time_label, 2, LV_PART_MAIN);

    // Week display - compact layout below time
    lv_obj_t* date_label = lv_label_create(time_container);
    R.ui.date = date_label;
    lv_label_set_text(date_label, "Monday");    // Use full weekday name to ensure sufficient width
    lv_obj_set_style_text_font(date_label, R.fonts.size_16.normal, LV_PART_MAIN);
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xBBBBBB), LV_PART_MAIN);
    lv_obj_set_style_text_align(date_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);

    // Main playback area - modern card design
    lv_obj_t* player_main = lv_obj_create(root);
    R.ui.player_group = player_main;
    lv_obj_remove_style_all(player_main);
    lv_obj_add_style(player_main, &R.styles.modern_card, LV_PART_MAIN);
    lv_obj_set_size(player_main, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(player_main, 32, LV_PART_MAIN);
    lv_obj_set_flex_flow(player_main, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(player_main, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(player_main, 1);

    // 3D effect album cover area + Gaussian blur background
    lv_obj_t* cover_section = lv_obj_create(player_main);
    lv_obj_remove_style_all(cover_section);
    lv_obj_set_size(cover_section, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cover_section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cover_section, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_margin_bottom(cover_section, 24, LV_PART_MAIN);

    // Album cover container - supports 3D rotation
    lv_obj_t* album_container = lv_obj_create(cover_section);
    R.ui.album_cover_container = album_container;
    lv_obj_remove_style_all(album_container);
    lv_obj_set_size(album_container, 320, 320);  // Enlarge container to accommodate multi-layer effects
    lv_obj_set_style_transform_pivot_x(album_container, 160, 0);
    lv_obj_set_style_transform_pivot_y(album_container, 160, 0);

    // Vinyl outer ring - outermost border effect
    lv_obj_t* vinyl_ring = lv_obj_create(album_container);
    R.ui.vinyl_ring = vinyl_ring;
    lv_obj_remove_style_all(vinyl_ring);
    lv_obj_add_style(vinyl_ring, &R.styles.vinyl_ring, LV_PART_MAIN);
    lv_obj_set_size(vinyl_ring, 320, 320);
    lv_obj_center(vinyl_ring);

    // Album cover image - perfect circular display, complete image without distortion
    lv_obj_t* album_cover = lv_image_create(album_container);
    R.ui.album_cover = album_cover;
    lv_obj_remove_style_all(album_cover);
    lv_obj_add_style(album_cover, &R.styles.circular_cover, LV_PART_MAIN);
    lv_obj_set_size(album_cover, 280, 280);
    lv_obj_center(album_cover);
    
    // Set image display mode: complete display, centered, maintain original aspect ratio
    lv_image_set_scale(album_cover, 256);  // Appropriate scaling to maintain clarity
    lv_image_set_inner_align(album_cover, LV_IMAGE_ALIGN_CENTER);
    lv_image_set_pivot(album_cover, 140, 140);  // Set rotation center point
    
    // Add image loading error handling
    lv_image_set_src(album_cover, R.images.nocover);
    
    // Ensure PNG images are displayed completely in circular container with original aspect ratio
    lv_obj_set_style_clip_corner(album_cover, true, LV_PART_MAIN);  // Enable circular clipping
    lv_obj_set_style_bg_img_recolor_opa(album_cover, LV_OPA_0, LV_PART_MAIN);  // Don't recolor
    lv_obj_set_style_image_recolor_opa(album_cover, LV_OPA_0, LV_PART_MAIN);  // Don't recolor image
    
    // Optimize image fill method to ensure complete display
    lv_obj_set_style_bg_image_opa(album_cover, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(album_cover, LV_RADIUS_CIRCLE, LV_PART_MAIN);  // Circular border

    // Remove vinyl center hole - user requested not to show center black dot
    // lv_obj_t* vinyl_center = lv_obj_create(album_container);
    // R.ui.vinyl_center = vinyl_center;
    R.ui.vinyl_center = NULL;  // Set to NULL to avoid references from other code

    // Song info area - scrolling text support
    lv_obj_t* song_info = lv_obj_create(player_main);
    lv_obj_remove_style_all(song_info);
    lv_obj_set_size(song_info, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(song_info, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(song_info, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_margin_bottom(song_info, 20, LV_PART_MAIN);

    // Song name - large font dynamic scrolling text
    lv_obj_t* song_title = lv_label_create(song_info);
    R.ui.album_name = song_title;
    lv_label_set_text(song_title, "Select Your Music");
    lv_label_set_long_mode(song_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(song_title, LV_PCT(90));
    lv_obj_set_style_text_align(song_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(song_title, R.fonts.size_28.normal, LV_PART_MAIN);  // Use 28pt font
    lv_obj_set_style_text_color(song_title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_margin_bottom(song_title, 12, LV_PART_MAIN);

    // Artist information - larger font
    lv_obj_t* artist_name = lv_label_create(song_info);
    R.ui.album_artist = artist_name;
    lv_label_set_text(artist_name, "Vela Music");
    lv_label_set_long_mode(artist_name, LV_LABEL_LONG_SCROLL);
    lv_obj_set_width(artist_name, LV_PCT(80));
    lv_obj_set_style_text_align(artist_name, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(artist_name, R.fonts.size_22.bold, LV_PART_MAIN);  // Increased from 16 to 22
    lv_obj_set_style_text_color(artist_name, lv_color_hex(0xE5E7EB), LV_PART_MAIN);

    // Playback progress area
    lv_obj_t* progress_section = lv_obj_create(player_main);
    R.ui.playback_group = progress_section;
    lv_obj_remove_style_all(progress_section);
    lv_obj_set_size(progress_section, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(progress_section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(progress_section, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_margin_bottom(progress_section, 24, LV_PART_MAIN);

    // Horizontal layout container for progress bar and time
    lv_obj_t* progress_bar_container = lv_obj_create(progress_section);
    lv_obj_remove_style_all(progress_bar_container);
    lv_obj_set_size(progress_bar_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(progress_bar_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(progress_bar_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_margin_bottom(progress_bar_container, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(progress_bar_container, 0, LV_PART_MAIN);

    // Gradient progress bar - occupies most of the width
    lv_obj_t* progress_bar = lv_bar_create(progress_bar_container);
    R.ui.playback_progress = progress_bar;
    lv_obj_remove_style_all(progress_bar);
    lv_obj_add_style(progress_bar, &R.styles.gradient_progress, LV_PART_INDICATOR);
    lv_obj_set_size(progress_bar, LV_PCT(65), 6);  // Adjust width to 65%, leaving more space for time display
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x374151), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(progress_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(progress_bar, 3, LV_PART_MAIN);

    // Time display area - right aligned, using larger font
    lv_obj_t* time_display = lv_spangroup_create(progress_bar_container);
    lv_span_t* current_time = lv_spangroup_new_span(time_display);
    lv_span_t* separator = lv_spangroup_new_span(time_display);
    lv_span_t* total_time = lv_spangroup_new_span(time_display);
    R.ui.playback_current_time = current_time;
    R.ui.playback_total_time = total_time;
    
    lv_span_set_text(current_time, "00:00");
    lv_span_set_text(separator, " / ");  // Use "/" separator, more concise
    lv_span_set_text(total_time, "00:00");
    lv_obj_set_style_text_font(time_display, R.fonts.size_22.bold, LV_PART_MAIN);  // Changed from 16pt to 22pt font
    lv_obj_set_style_text_color(time_display, lv_color_hex(0x3B82F6), LV_PART_MAIN);
    lv_obj_set_style_text_align(time_display, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);  // Right aligned
    lv_obj_set_style_margin_right(time_display, 8, LV_PART_MAIN);  // Right margin
    lv_style_set_text_color(&separator->style, lv_color_hex(0x9CA3AF));
    lv_style_set_text_color(&total_time->style, lv_color_hex(0x9CA3AF));

    // Multi-function control area - horizontal distributed layout with increased spacing
    lv_obj_t* control_area = lv_obj_create(player_main);
    lv_obj_remove_style_all(control_area);
    lv_obj_set_size(control_area, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(control_area, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(control_area, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);  // Evenly distributed layout
    lv_obj_set_style_pad_all(control_area, 20, LV_PART_MAIN);  // Increase padding from 12 to 20
    lv_obj_set_style_pad_column(control_area, 16, LV_PART_MAIN);  // Set column spacing between buttons

    // Professional playlist button - apply professional configuration
    lv_obj_t* playlist_btn = lv_button_create(control_area);
    lv_obj_t* playlist_icon = lv_image_create(playlist_btn);
    lv_obj_remove_style_all(playlist_btn);
    
    // Apply professional button configuration
    // Standard button configuration
    lv_obj_set_style_bg_color(playlist_btn, lv_color_hex(0x374151), LV_PART_MAIN);
    lv_obj_set_style_bg_color(playlist_btn, lv_color_hex(0x4B5563), LV_PART_MAIN | LV_STATE_PRESSED);
    
    lv_image_set_src(playlist_icon, R.images.playlist);
    lv_obj_set_size(playlist_icon, 28, 28);
    lv_obj_center(playlist_icon);

    // Professional previous button
    lv_obj_t* prev_btn = lv_button_create(control_area);
    lv_obj_t* prev_icon = lv_image_create(prev_btn);
    lv_obj_remove_style_all(prev_btn);
    
    // Apply professional button configuration
    // Previous button configuration
    lv_obj_set_style_bg_color(prev_btn, lv_color_hex(0x374151), LV_PART_MAIN);
    lv_obj_set_style_bg_color(prev_btn, lv_color_hex(0x4B5563), LV_PART_MAIN | LV_STATE_PRESSED);
    
    lv_image_set_src(prev_icon, R.images.previous);
    lv_obj_set_size(prev_icon, 32, 32);
    lv_obj_center(prev_icon);

    // Simplified version: remove 10s rewind button, keep interface clean

    // Professional main play button - special glow effect
    lv_obj_t* play_btn = lv_button_create(control_area);
    lv_obj_t* play_icon = lv_image_create(play_btn);
    R.ui.play_btn = play_icon;
    lv_obj_remove_style_all(play_btn);
    
    // Apply professional button configuration - main button uses large size
    // Play button configuration
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0x374151), LV_PART_MAIN);
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0x4B5563), LV_PART_MAIN | LV_STATE_PRESSED);
    
    // Main button special glow effect
    lv_obj_set_style_shadow_width(play_btn, 25, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(play_btn, 35, LV_STATE_PRESSED);
    lv_obj_set_style_shadow_color(play_btn, lv_color_hex(0x00BFFF), LV_PART_MAIN);  // Blue shadow
    lv_obj_set_style_shadow_opa(play_btn, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(play_btn, LV_OPA_90, LV_STATE_PRESSED);
    
    lv_image_set_src(play_icon, R.images.play);
    lv_obj_set_size(play_icon, 48, 48);
    lv_obj_center(play_icon);

    // Simplified version: remove 10s fast forward button, keep interface clean

    // Pro next button
    lv_obj_t* next_btn = lv_button_create(control_area);
    lv_obj_t* next_icon = lv_image_create(next_btn);
    lv_obj_remove_style_all(next_btn);
    
    // Apply professional button configuration
    // Next button configuration
    lv_obj_set_style_bg_color(next_btn, lv_color_hex(0x374151), LV_PART_MAIN);
    lv_obj_set_style_bg_color(next_btn, lv_color_hex(0x4B5563), LV_PART_MAIN | LV_STATE_PRESSED);
    
    lv_image_set_src(next_icon, R.images.next);
    lv_obj_set_size(next_icon, 32, 32);
    lv_obj_center(next_icon);

    // Pro volume button
    lv_obj_t* volume_btn = lv_button_create(control_area);
    lv_obj_t* volume_icon = lv_image_create(volume_btn);
    R.ui.audio = volume_icon;
    lv_obj_remove_style_all(volume_btn);
    
    // Apply professional button configuration
    // Volume button configuration
    lv_obj_set_style_bg_color(volume_btn, lv_color_hex(0x374151), LV_PART_MAIN);
    lv_obj_set_style_bg_color(volume_btn, lv_color_hex(0x4B5563), LV_PART_MAIN | LV_STATE_PRESSED);
    
    lv_obj_set_size(volume_icon, 28, 28);
    lv_obj_center(volume_icon);

    // Create top layer overlay (volume bar, playlist, etc)
    app_create_top_layer();

    // Pro button interaction applied through configuration system
    // UI interaction optimization applied

    // Pro event binding - supports multiple interaction modes and enhanced feedback
    lv_obj_add_event_cb(playlist_btn, app_playlist_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(playlist_btn, app_playlist_event_handler, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(playlist_btn, app_playlist_event_handler, LV_EVENT_RELEASED, NULL);
    
    lv_obj_add_event_cb(volume_btn, app_audio_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(volume_btn, app_audio_event_handler, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(volume_btn, app_audio_event_handler, LV_EVENT_RELEASED, NULL);
    
    lv_obj_add_event_cb(play_btn, app_play_status_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(play_btn, app_play_status_event_handler, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(play_btn, app_play_status_event_handler, LV_EVENT_RELEASED, NULL);
    
    lv_obj_add_event_cb(prev_btn, app_switch_album_event_handler, LV_EVENT_CLICKED, (lv_uintptr_t*)SWITCH_ALBUM_MODE_PREV);
    lv_obj_add_event_cb(prev_btn, app_switch_album_event_handler, LV_EVENT_PRESSED, (lv_uintptr_t*)SWITCH_ALBUM_MODE_PREV);
    lv_obj_add_event_cb(prev_btn, app_switch_album_event_handler, LV_EVENT_RELEASED, (lv_uintptr_t*)SWITCH_ALBUM_MODE_PREV);
    
    lv_obj_add_event_cb(next_btn, app_switch_album_event_handler, LV_EVENT_CLICKED, (lv_uintptr_t*)SWITCH_ALBUM_MODE_NEXT);
    lv_obj_add_event_cb(next_btn, app_switch_album_event_handler, LV_EVENT_PRESSED, (lv_uintptr_t*)SWITCH_ALBUM_MODE_NEXT);
    lv_obj_add_event_cb(next_btn, app_switch_album_event_handler, LV_EVENT_RELEASED, (lv_uintptr_t*)SWITCH_ALBUM_MODE_NEXT);
    
    // Add long press support - environment friendly
    lv_obj_add_event_cb(prev_btn, app_switch_album_event_handler, LV_EVENT_LONG_PRESSED_REPEAT, (lv_uintptr_t*)SWITCH_ALBUM_MODE_PREV);
    lv_obj_add_event_cb(next_btn, app_switch_album_event_handler, LV_EVENT_LONG_PRESSED_REPEAT, (lv_uintptr_t*)SWITCH_ALBUM_MODE_NEXT);
    
    // Enhanced progress bar interaction - supports click jump and drag
    lv_obj_add_event_cb(progress_bar, app_playback_progress_bar_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_set_ext_click_area(progress_bar, 15);  // Expand progress bar touch area to improve drag experience
    
    // Enhanced volume bar interaction
    lv_obj_set_ext_click_area(R.ui.volume_bar, 10);  // Extend volume bar click area
    
    // Button long press delay set through pro configuration system
    // Event binding completed

    // Start time update timer
    app_start_updating_date_time();
    
    // UI creation completed
}

// Time update functionality
static void app_start_updating_date_time(void)
{
    // Ensure time and date UI components are created
    if (!R.ui.time || !R.ui.date) {
        LV_LOG_ERROR("Time/Date UI components not ready, cannot start timer");
        return;
    }

    LV_LOG_USER("Starting date/time update system...");
    
    // Update time immediately once
    app_refresh_date_time();
    
    // Check if timer already exists before creating
    if (C.timers.refresh_date_time != NULL) {
        lv_timer_delete(C.timers.refresh_date_time);
        C.timers.refresh_date_time = NULL;
    }
    
    C.timers.refresh_date_time = lv_timer_create(app_refresh_date_time_timer_cb, 1000, NULL);
    if (C.timers.refresh_date_time == NULL) {
        LV_LOG_ERROR("Failed to create date/time update timer");
        return;
    }
    
    LV_LOG_USER("Date/Time update timer created successfully - updating every 1000ms");
}

static void read_configs(void)
{
    uint32_t file_size;
    lv_fs_file_t file;
    lv_fs_open(&file, RES_ROOT "/config.json", LV_FS_MODE_RD);

    lv_fs_seek(&file, 0, LV_FS_SEEK_END);
    lv_fs_tell(&file, &file_size);
    lv_fs_seek(&file, 0, LV_FS_SEEK_SET);

    char* buff = lv_malloc(file_size);
    lv_fs_read(&file, buff, file_size, NULL);

    const char* json_string = buff;

    cJSON* json = cJSON_Parse(json_string);
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            LV_LOG_ERROR("parse error: %p", error_ptr);
        }
        lv_free(buff);
        return;
    }

#if WIFI_ENABLED
    cJSON* wifi_object = cJSON_GetObjectItem(json, "wifi");

    const char* ssid = cJSON_GetStringValue(cJSON_GetObjectItem(wifi_object, "ssid"));
    const char* pswd = cJSON_GetStringValue(cJSON_GetObjectItem(wifi_object, "pswd"));
    const int version = cJSON_GetNumberValue(cJSON_GetObjectItem(wifi_object, "wpa_ver"));

    lv_strcpy(CF.wifi.ssid, ssid);
    lv_strcpy(CF.wifi.pswd, pswd);
    CF.wifi.ver_flag = version;
#endif

    cJSON_Delete(json);

    lv_free(buff);
}

static void reload_music_config(void)
{
    // Safe memory cleanup - enhanced version
    LV_LOG_USER("Starting to reload music configuration...");
    
    /* Clear previous music config */
    if (R.albums) {
        for (int i = 0; i < R.album_count; i++) {
            // Safely release string memory
            if (R.albums[i].name) {
                lv_free((void*)R.albums[i].name);
                R.albums[i].name = NULL;
            }
            if (R.albums[i].artist) {
                lv_free((void*)R.albums[i].artist);
                R.albums[i].artist = NULL;
            }
            // Clear path and cover character arrays
            memset(R.albums[i].path, 0, sizeof(R.albums[i].path));
            memset(R.albums[i].cover, 0, sizeof(R.albums[i].cover));
        }

        lv_free(R.albums);
        R.albums = NULL;
    }
    
    R.album_count = 0;
    
    // Memory statistics
    // Memory cleanup completed - silent mode

    /* Load music config - enhanced memory safety */
    uint32_t file_size;
    lv_fs_file_t file;
    char* buff = NULL;
    cJSON* json = NULL;
    
    // Safe file opening
    lv_fs_res_t res = lv_fs_open(&file, MUSICS_ROOT "/manifest.json", LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) {
        LV_LOG_ERROR("Cannot open music manifest file: %s", MUSICS_ROOT "/manifest.json");
        return;
    }

    // Safe file size retrieval
    if (lv_fs_seek(&file, 0, LV_FS_SEEK_END) != LV_FS_RES_OK ||
        lv_fs_tell(&file, &file_size) != LV_FS_RES_OK ||
        lv_fs_seek(&file, 0, LV_FS_SEEK_SET) != LV_FS_RES_OK) {
        LV_LOG_ERROR("File operation failed");
        lv_fs_close(&file);
        return;
    }

    // Safe memory allocation
    if (file_size == 0 || file_size > 2 * 1024 * 1024) {  // Limit 2MB
        LV_LOG_ERROR("Music manifest file size abnormal: %lu bytes", (unsigned long)file_size);
        lv_fs_close(&file);
        return;
    }
    
    buff = lv_malloc(file_size + 1);  // +1 for null terminator
    if (!buff) {
        // Memory allocation failed - silent handling
        lv_fs_close(&file);
        return;
    }
    
    memset(buff, 0, file_size + 1);
    
    uint32_t bytes_read;
    if (lv_fs_read(&file, buff, file_size, &bytes_read) != LV_FS_RES_OK || bytes_read != file_size) {
        LV_LOG_ERROR("File read failed: expected %lu bytes, actually read %lu bytes", 
                     (unsigned long)file_size, (unsigned long)bytes_read);
        goto cleanup_and_exit;
    }
    
    lv_fs_close(&file);

    // JSON parsing
    json = cJSON_Parse(buff);
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        LV_LOG_ERROR("JSON parsing error: %s", error_ptr ? error_ptr : "unknown error");
        goto cleanup_and_exit;
    }

    cJSON* musics_object = cJSON_GetObjectItem(json, "musics");
    if (musics_object == NULL) {
        LV_LOG_ERROR("'musics' object not found in JSON");
        goto cleanup_and_exit;
    }

    int total_albums = cJSON_GetArraySize(musics_object);
    // Limit album count to prevent memory overflow
    #define MAX_SAFE_ALBUMS 50
    R.album_count = (total_albums > MAX_SAFE_ALBUMS) ? MAX_SAFE_ALBUMS : total_albums;
    
    LV_LOG_USER("Found %d albums in JSON, loading %d", total_albums, R.album_count);
    
    R.albums = lv_malloc_zeroed(R.album_count * sizeof(album_info_t));
    if (!R.albums) {
        // Album memory allocation failed
        lv_free(buff);
        return;
    }

    for (int i = 0; i < R.album_count; i++) {
        cJSON* music_object = cJSON_GetArrayItem(musics_object, i);
        if (!music_object) {
            // Album JSON object invalid
            continue;
        }

        const char* path = cJSON_GetStringValue(cJSON_GetObjectItem(music_object, "path"));
        const char* name = cJSON_GetStringValue(cJSON_GetObjectItem(music_object, "name"));
        const char* artist = cJSON_GetStringValue(cJSON_GetObjectItem(music_object, "artist"));
        const char* cover = cJSON_GetStringValue(cJSON_GetObjectItem(music_object, "cover"));
        const double total_time_double = cJSON_GetNumberValue(cJSON_GetObjectItem(music_object, "total_time"));
        const char* color_str = cJSON_GetStringValue(cJSON_GetObjectItem(music_object, "color"));

        // Safety check
        if (!path || !name) {
            // Album missing necessary information
            continue;
        }

        uint64_t total_time = (uint64_t)total_time_double;
        LV_LOG_USER("Loading album %d: %s - %s", i, name, artist ? artist : "Unknown Artist");
        uint32_t color_int = strtoul(color_str + 1, NULL, 16);

        if (total_time == 0)
            total_time = 1;

        lv_color_t color = lv_color_hex(color_int);

        lv_snprintf(R.albums[i].path, sizeof(R.albums[i].path), "%s/%s", MUSICS_ROOT, path);
        lv_snprintf(R.albums[i].cover, sizeof(R.albums[i].cover), "%s/%s", MUSICS_ROOT, cover);
        R.albums[i].name = lv_strdup(name);
        R.albums[i].artist = lv_strdup(artist);
        R.albums[i].total_time = total_time;
        R.albums[i].color = color;

        LV_LOG_USER("Album %d: %s - %s | %s %s %lu", i, R.albums[i].name, R.albums[i].artist, R.albums[i].path, R.albums[i].cover, (unsigned long)total_time);
    }

    // Safe resource cleanup
    goto cleanup_success;

cleanup_and_exit:
    // Resource release when cleanup fails
    if (R.albums) {
        lv_free(R.albums);
        R.albums = NULL;
        R.album_count = 0;
    }
    
cleanup_success:
    // Resources that need to be cleaned up regardless of success or failure
    if (json) {
        cJSON_Delete(json);
        json = NULL;
    }
    
    if (buff) {
        lv_free(buff);
        buff = NULL;
    }
    
    // Music configuration loading completed
}

