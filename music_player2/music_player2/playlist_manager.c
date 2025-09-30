/**
 * Playlist Manager
 * Handles playlist UI and user interactions
 */

#include "music_player.h"
#include "playlist_manager.h"
#include "font_config.h"
#include <stdio.h>

// External variables
extern struct resource_s R;
extern struct ctx_s C;

// Defines
#define MAX_PLAYLIST_ITEMS 6
#define PLAYLIST_ITEM_HEIGHT 70
#define PLAYLIST_NO_ANIMATION
// Variables
static lv_obj_t* playlist_container = NULL;
static bool playlist_is_open = false;

// Functions
static void create_playlist_item(lv_obj_t* parent, int index);
static void playlist_item_click_cb(lv_event_t* e);
static void playlist_close_cb(lv_event_t* e);
static void create_playlist_item(lv_obj_t* parent, int index) {
    if (index >= R.album_count || index >= MAX_PLAYLIST_ITEMS) {
        return;
    }
    
    // Create simple button for playlist item
    lv_obj_t* btn = lv_btn_create(parent);
    if (!btn) return;
    
    lv_obj_set_size(btn, LV_PCT(100), PLAYLIST_ITEM_HEIGHT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x4A4A4A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_margin_bottom(btn, 4, LV_PART_MAIN);
    
    // Create label with enhanced styling
    lv_obj_t* label = lv_label_create(btn);
    if (!label) return;
    
    char text[80];
    const char* name = R.albums[index].name ? R.albums[index].name : "Unknown";
    snprintf(text, sizeof(text), "%d. %s", index + 1, name);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    
    // Apply playlist font
    const lv_font_t* playlist_font = get_playlist_font("song");
    if (playlist_font) {
        lv_obj_set_style_text_font(label, playlist_font, LV_PART_MAIN);
    }
    
    // Set text color to white
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    // Click event handler
    lv_obj_add_event_cb(btn, playlist_item_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)index);
}

/**
 * Playlist item click handler
 */
static void playlist_item_click_cb(lv_event_t* e) {
    int index = (int)(uintptr_t)lv_event_get_user_data(e);
    
    if (index >= 0 && index < R.album_count) {
        // Switch to selected track
        extern void app_switch_to_album(int index);
        app_switch_to_album(index);
        
        // Close playlist
        playlist_manager_close();
    }
}

/**
 * Close playlist callback
 */
static void playlist_close_cb(lv_event_t* e) {
    playlist_manager_close();
}

/*********************
 * GLOBAL FUNCTIONS
 *********************/

/**
 * Create playlist
 */
void playlist_manager_create(lv_obj_t* parent) {
    if (playlist_container || !parent) {
        return;
    }
    
    // Create fullscreen container
    playlist_container = lv_obj_create(parent);
    if (!playlist_container) {
        return;
    }
    
    lv_obj_remove_style_all(playlist_container);
    lv_obj_set_size(playlist_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(playlist_container, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(playlist_container, LV_OPA_80, LV_PART_MAIN);
    
    // Close on background click
    lv_obj_add_flag(playlist_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(playlist_container, playlist_close_cb, LV_EVENT_CLICKED, NULL);
    
    // Create content area
    lv_obj_t* content = lv_obj_create(playlist_container);
    if (!content) {
        lv_obj_del(playlist_container);
        playlist_container = NULL;
        return;
    }
    
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, LV_PCT(90), LV_PCT(80));
    lv_obj_center(content);
    lv_obj_set_style_bg_color(content, lv_color_hex(0x222222), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(content, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(content, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(content, 8, LV_PART_MAIN);
    
    // Prevent event propagation
    lv_obj_remove_flag(content, LV_OBJ_FLAG_EVENT_BUBBLE);
    
    // Enhanced title styling
    lv_obj_t* title = lv_label_create(content);
    if (title) {
        lv_label_set_text(title, "Playlist");
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_margin_bottom(title, 12, LV_PART_MAIN);
        
        // Apply playlist title font
        const lv_font_t* title_font = get_playlist_font("title");
        if (title_font) {
            lv_obj_set_style_text_font(title, title_font, LV_PART_MAIN);
        }
        
        // Center align title
        lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }
    
    // Create limited list items
    int max_items = (R.album_count < MAX_PLAYLIST_ITEMS) ? R.album_count : MAX_PLAYLIST_ITEMS;
    for (int i = 0; i < max_items; i++) {
        create_playlist_item(content, i);
    }
    
    // Mark playlist as open
    playlist_is_open = true;
}

/**
 * Close playlist
 */
void playlist_manager_close(void) {
    if (playlist_container) {
        lv_obj_del(playlist_container);
        playlist_container = NULL;
    }
    
    // Mark playlist as closed
    playlist_is_open = false;
}

/**
 * Check if playlist is open
 */
bool playlist_manager_is_open(void) {
    return playlist_is_open;
}

/**
 * Refresh playlist - simple recreation logic
 */
void playlist_manager_refresh(void) {
    if (playlist_is_open) {
        // If already open, close and recreate
        lv_obj_t* parent = lv_obj_get_parent(playlist_container);
        playlist_manager_close();
        if (parent) {
            playlist_manager_create(parent);
        }
    }
}