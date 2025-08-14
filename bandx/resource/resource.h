/**
 * @file resource.h
 *
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <lvgl/lvgl.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the resource manager.
 */
void resource_init(void);

/**
 * Get a font resource by key.
 * @param key The key associated with the desired font.
 * @return A pointer to the font resource, or NULL if not found.
 */
const lv_font_t* resource_get_font(const char* key);

/**
 * Get an image resource by key.
 * @param key The key associated with the desired image.
 * @return A pointer to the image resource, or NULL if not found.
 */
const void* resource_get_img(const char* key);

/**
 * Get a style resource by key.
 * @param key The key associated with the desired style.
 * @return A pointer to the style resource, or NULL if not found.
 */
lv_style_t* resource_get_style(const char* key);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*RESOURCE_H*/
