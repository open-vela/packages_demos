/**
 * @file lv_anim_label.h
 *
 */

#ifndef LV_ANIM_LABEL_H
#define LV_ANIM_LABEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t obj;
    lv_obj_t* label_1;
    lv_obj_t* label_2;
    lv_obj_t* label_act;
    lv_dir_t enter_dir;
    lv_dir_t exit_dir;
    lv_anim_t a_enter;
    lv_anim_t a_exit;
    uint32_t duration;
    lv_anim_path_cb_t path_cb;
} lv_anim_label_t;

extern const lv_obj_class_t lv_anim_label_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a new animation label object.
 * @param parent Pointer to the parent object.
 * @return A pointer to the newly created animation label object.
 */
lv_obj_t* lv_anim_label_create(lv_obj_t* parent);

/**
 * Set the direction of the animation label.
 * @param obj Pointer to the animation label object.
 * @param dir The direction to set for the animation.
 */
void lv_anim_label_set_dir(lv_obj_t* obj, lv_dir_t dir);

/**
 * Set the entry direction for the animation label.
 * @param obj Pointer to the animation label object.
 * @param dir The entry direction to set.
 */
void lv_anim_label_set_enter_dir(lv_obj_t* obj, lv_dir_t dir);

/**
 * Set the exit direction for the animation label.
 * @param obj Pointer to the animation label object.
 * @param dir The exit direction to set.
 */
void lv_anim_label_set_exit_dir(lv_obj_t* obj, lv_dir_t dir);

/**
 * Set the duration of the animation for the animation label.
 * @param obj Pointer to the animation label object.
 * @param duration The duration of the animation in milliseconds.
 */
void lv_anim_label_set_time(lv_obj_t* obj, uint32_t duration);

/**
 * Set the animation path callback for the animation label.
 * @param obj Pointer to the animation label object.
 * @param path_cb Callback function for the animation path.
 */
void lv_anim_label_set_path(lv_obj_t* obj, lv_anim_path_cb_t path_cb);

/**
 * Add a style to the animation label.
 * @param obj Pointer to the animation label object.
 * @param style Pointer to the style to add.
 */
void lv_anim_label_add_style(lv_obj_t* obj, lv_style_t* style);

/**
 * Set a custom enter animation for the animation label.
 * @param obj Pointer to the animation label object.
 * @param a Pointer to the custom animation.
 */
void lv_anim_label_set_custom_enter_anim(lv_obj_t* obj, const lv_anim_t* a);

/**
 * Set a custom exit animation for the animation label.
 * @param obj Pointer to the animation label object.
 * @param a Pointer to the custom animation.
 */
void lv_anim_label_set_custom_exit_anim(lv_obj_t* obj, const lv_anim_t* a);

/**
 * Push new text to the animation label.
 * @param obj Pointer to the animation label object.
 * @param txt The text to push to the animation label.
 */
void lv_anim_label_push_text(lv_obj_t* obj, const char* txt);

/**
 * Get the enter direction of the animation label.
 * @param obj Pointer to the animation label object.
 * @return The current enter direction of the animation label.
 */
lv_dir_t lv_anim_label_get_enter_dir(lv_obj_t* obj);

/**
 * Get the exit direction of the animation label.
 * @param obj Pointer to the animation label object.
 * @return The current exit direction of the animation label.
 */
lv_dir_t lv_anim_label_get_exit_dir(lv_obj_t* obj);

/**
 * Get the duration of the animation for the animation label.
 * @param obj Pointer to the animation label object.
 * @return The duration of the animation in milliseconds.
 */
uint32_t lv_anim_label_get_time(lv_obj_t* obj);

/**
 * Get the animation path callback for the animation label.
 * @param obj Pointer to the animation label object.
 * @return The callback function for the animation path.
 */
lv_anim_path_cb_t lv_anim_label_get_path(lv_obj_t* obj);

/**
 * Get the current text of the animation label.
 * @param obj Pointer to the animation label object.
 * @return The current text of the animation label.
 */
const char* lv_anim_label_get_text(lv_obj_t* obj);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_ANIM_LABEL_H*/