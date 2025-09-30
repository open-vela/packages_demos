//
// Vela Music Player - Minimalist Playlist Manager Header File
// Created by Vela on 2025/8/20
// Lightweight playlist implementation based on emergency_playlist.c
//

#ifndef PLAYLIST_MANAGER_H
#define PLAYLIST_MANAGER_H

#include "lvgl.h"
#include <stdbool.h>

/*********************
 * GLOBAL PROTOTYPES
 *********************/

/**
 * @brief Create lightweight playlist interface (based on emergency mode)
 * @param parent Parent container
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
 * @brief Whether playlist is open
 * @return true if open, false if closed
 */
bool playlist_manager_is_open(void);

#endif // PLAYLIST_MANAGER_H