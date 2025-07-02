#ifndef __RELATION_CAL_H__
#define __RELATION_CAL_H__

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

#define MAX_REL_LEN 256

#define FATHER 0
#define HUSBAND 1
#define SON 2
#define ELDER_BRO 3
#define ELDER_SIS 4
#define DELETE 5
#define CLEAR 6
#define MOTHER 7
#define WIFI 8
#define DAUGHTER 9
#define YOUNGER_BRO 10
#define YOUNGER_SIS 11
#define MY_NULL 12
#define CALCULATED 13

#define REL_CAL_RES_ROOT CONFIG_LVX_REL_CAL_DATA_ROOT "/res"
#define REL_CAL_FONTS_ROOT REL_CAL_RES_ROOT "/fonts"

typedef enum relation_cal_status_e
{
    STATUS_INIT,
    STATUS_INPUTTING,
    STATUS_CALCULATED,
    STATUS_XXX
} relation_cal_status_t;

typedef enum gender_e
{
    GENDER_MALE,
    GENDER_FEMALE,
    GENDER_UNKNOWN
} gender_t;

typedef enum relation_type_e
{
    REL_SELF,                             // Self
    REL_FATHER,                           // Father
    REL_MOTHER,                           // Mother
    REL_SON,                              // Son
    REL_DAUGHTER,                         // Daughter
    REL_WIFE,                             // Wife
    REL_HUSBAND,                          // Husband
    REL_ELDER_BROTHER,                    // Elder brother
    REL_YOUNGER_BROTHER,                  // Younger brother
    REL_ELDER_SISTER,                     // Elder sister
    REL_YOUNGER_SISTER,                   // Younger sister
    REL_GRANDFATHER,                      // Paternal grandfather
    REL_GRANDMOTHER,                      // Paternal grandmother
    REL_UNCLE,                            // Uncle (father's elder brother)
    REL_AUNT,                             // Aunt (father's sister)
    REL_COUSIN_MALE,                      // Paternal male cousin
    REL_COUSIN_FEMALE,                    // Paternal female cousin
    REL_GREAT_GRANDFATHER,                // Paternal great - grandfather
    REL_GREAT_GRANDMOTHER,                // Paternal great - grandmother
    REL_MATERNAL_GRANDFATHER,             // Maternal grandfather
    REL_MATERNAL_GRANDMOTHER,             // Maternal grandmother
    REL_MATERNAL_GREAT_GRANDFATHER,       // Maternal great - grandfather
    REL_MATERNAL_GREAT_GRANDMOTHER,       // Maternal great - grandmother
    REL_FIRST_COUSIN_ONCE_REMOVED_MALE,   // First cousin once removed (male)
    REL_FIRST_COUSIN_ONCE_REMOVED_FEMALE, // First cousin once removed (female)
    REL_SECOND_COUSIN_MALE,               // Second male cousin
    REL_SECOND_COUSIN_FEMALE,             // Second female cousin
    REL_GREAT_UNCLE,                      // Granduncle (grandfather's younger brother)
    REL_GREAT_AUNT,                       // Grandaunt (grandfather's sister)
    REL_MATERNAL_UNCLE,                   // Maternal uncle (mother's brother)
    REL_MATERNAL_AUNT,                    // Maternal aunt (mother's sister)
    REL_NEPHEW,                           // Nephew
    REL_NIECE,                            // Niece
    REL_SON_IN_LAW,                       // Son - in - law
    REL_DAUGHTER_IN_LAW,                  // Daughter - in - law
    REL_GRANDSON,                         // Grandson
    REL_GRANDDAUGHTER,                    // Granddaughter
    REL_GREAT_GRANDSON,                   // Great - grandson
    REL_GREAT_GRANDDAUGHTER,              // Great - granddaughter
    REL_COUSIN_SON,                       // Cousin's son
    REL_COUSIN_DAUGHTER,                  // Cousin's daughter
    REL_GRAND_NEPHEW,                     // Grand - nephew (brother's grandson)
    REL_GRAND_NIECE,                      // Grand - niece (brother's granddaughter)
    REL_NULL                              // Unknown
} relation_type_t;

typedef struct btnm_relation_s
{
    const char *btnm_text;
    relation_type_t relation;
    gender_t gender;
} btnm_relation_t;

typedef struct relation_call_s
{
    const char *call;
    relation_type_t relation;
} relation_call_t;

typedef struct relation_cal_s
{
    struct
    {
        lv_obj_t *title;
        lv_obj_t *note;
        lv_obj_t *btnm;
        lv_obj_t *screen;
    } ui;

    struct
    {
        const lv_font_t *siyuan;
    } fonts;

    relation_type_t rel_list[MAX_REL_LEN];
    int rel_count;
    relation_cal_status_t status;

} relation_cal_t;

typedef struct relation_transformation_s
{
    relation_type_t from;
    relation_type_t to;
    relation_type_t result;
} relation_transformation_t;

void relation_cal_app_create(void);

#endif /* __RELATION_CAL_H__ */
