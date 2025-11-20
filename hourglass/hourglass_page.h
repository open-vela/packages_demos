#ifndef HOURGLASS_PAGE_H
#define HOURGLASS_PAGE_H

#include <string.h>
#include <stdio.h>
#include "lvgl.h"

typedef struct post1 {
    lv_obj_t *obj;
    bool state;
}Post1_t;

void sandglass_page_create(void);

#endif