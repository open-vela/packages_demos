/**
 * @file page.h
 *
 */

#ifndef PAGE_H
#define PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

#ifdef CONFIG_LVX_USE_DEMO_BANDX

#include "../resource/resource.h"
#include "../utils/lv_auto_event.h"

/*********************
 *      DEFINES
 *********************/

#define PAGE_CLASS_DEF(NAME)                         \
    const lv_fragment_class_t _page_##NAME##_cls = { \
        .constructor_cb = on_page_construct,         \
        .destructor_cb = on_page_destruct,           \
        .attached_cb = on_page_attached,             \
        .detached_cb = on_page_detached,             \
        .create_obj_cb = on_page_create,             \
        .obj_created_cb = on_page_created,           \
        .obj_will_delete_cb = on_page_will_delete,   \
        .obj_deleted_cb = on_page_deleted,           \
        .event_cb = on_page_event,                   \
        .instance_size = sizeof(page_ctx_t)          \
    }

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the page manager.
 */
void page_init(void);

/**
 * Create a new page fragment.
 * @param name The name of the page.
 * @param arg Arguments to be passed for the page fragment create.
 * @return A pointer to the newly created page fragment.
 */
lv_fragment_t* page_create(const char* name, void* arg);

/**
 * Push a new page onto the stack.
 * @param self Pointer to the current page fragment.
 * @param name The name of the page to push.
 * @param arg Arguments for the new page.
 * @return Return true if the push operation was successful.
 */
bool page_push(lv_fragment_t* self, const char* name, void* arg);

/**
 * Pop the current page from the stack.
 * @param self Pointer to the current page fragment.
 */
void page_pop(lv_fragment_t* self);

/**
 * Get the auto-show enable status.
 * @return Return true if auto-show is enabled, false otherwise.
 */
bool page_get_autoshow_enable(void);

/**
 * Set the auto-show enable status.
 * @param en Enable or disable the auto-show.
 */
void page_set_autoshow_enable(bool en);

/**
 * Set the status of the last page dial plate.
 * @param value Set to true to indicate that the last page is dial plate, false otherwise.
 */
void page_set_last_page_dialplate(bool value);

/**
 * Check if the last page is dial plate.
 * @return Return true if the last page is dial plate, false otherwise.
 */
bool page_is_last_page_dialplate(void);

/**********************
 *      MACROS
 **********************/
#define PAGE_VER_RES 368
#define PAGE_HOR_RES 194

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#define AUTO_EVENT_CREATE(ae, ae_data)                               \
    do {                                                             \
        if (page_get_autoshow_enable()) {                            \
            ae = lv_auto_event_create(ae_data, ARRAY_SIZE(ae_data)); \
        } else {                                                     \
            ae = NULL;                                               \
        }                                                            \
    } while (0)

#define AUTO_EVENT_DELETE(ae)             \
    do {                                  \
        if (page_get_autoshow_enable()) { \
            lv_auto_event_del(ae);        \
            ae = NULL;                    \
        }                                 \
    } while (0)

#endif /*CONFIG_LVX_USE_DEMO_BANDX*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*PAGE_H*/
