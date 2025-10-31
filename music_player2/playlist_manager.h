#ifndef PLAYLIST_MANAGER_H
#define PLAYLIST_MANAGER_H

#include "lvgl.h"
#include <stdbool.h>

void playlist_manager_create(lv_obj_t* parent);
void playlist_manager_refresh(void);
void playlist_manager_close(void);
bool playlist_manager_is_open(void);

#endif