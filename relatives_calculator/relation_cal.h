#ifndef __RRLATION_CAL_H__
#define __RRLATION_CAL_H__

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_REL_LEN 256

typedef enum relation_cal_status_e {
    STATUS_INIT,
    STATUS_INPUTTING,
    STATUS_CALCULATED,
    STATUS_XXX
}relation_cal_status_t;

typedef enum gender_e {
    GENDER_MALE,
    GENDER_FEMALE,
    GENDER_UNKNOWN
}gender_t;

typedef enum relation_type_e {
    REL_SELF,                        // 自己
    REL_FATHER,                      // 父亲
    REL_MOTHER,                      // 母亲
    REL_SON,                         // 儿子
    REL_DAUGHTER,                    // 女儿
    REL_WIFE,                        // 妻子
    REL_HUSBAND,                     // 丈夫
    REL_ELDER_BROTHER,               // 哥哥
    REL_YOUNGER_BROTHER,             // 弟弟
    REL_ELDER_SISTER,                // 姐姐
    REL_YOUNGER_SISTER,              // 妹妹
    REL_GRANDFATHER,                 // 祖父
    REL_GRANDMOTHER,                 // 祖母
    REL_UNCLE,                       // 伯父（父亲的哥哥）
    REL_AUNT,                        // 姑母（父亲的姐妹）
    REL_COUSIN_MALE,                 // 堂兄弟（父系）
    REL_COUSIN_FEMALE,               // 堂姐妹（父系）
    REL_GREAT_GRANDFATHER,           // 曾祖父（父系）
    REL_GREAT_GRANDMOTHER,           // 曾祖母（父系）
    REL_MATERNAL_GRANDFATHER,        // 外祖父
    REL_MATERNAL_GRANDMOTHER,        // 外祖母
    REL_MATERNAL_GREAT_GRANDFATHER,  // 外曾祖父
    REL_MATERNAL_GREAT_GRANDMOTHER,  // 外曾祖母
    REL_FIRST_COUSIN_ONCE_REMOVED_MALE,   // 表侄
    REL_FIRST_COUSIN_ONCE_REMOVED_FEMALE, // 表侄女
    REL_SECOND_COUSIN_MALE,          // 再从兄弟
    REL_SECOND_COUSIN_FEMALE,        // 再从姐妹
    REL_GREAT_UNCLE,                 // 叔祖父（祖父的弟弟）
    REL_GREAT_AUNT,                  // 姑祖母（祖父的姐妹）
    REL_MATERNAL_UNCLE,              // 舅父（母亲的兄弟）
    REL_MATERNAL_AUNT,               // 姨母（母亲的姐妹）
    REL_NEPHEW,                      // 侄子
    REL_NIECE,                       // 侄女
    REL_SON_IN_LAW,                  // 媳妇
    REL_DAUGHTER_IN_LAW,             // 女婿
    REL_GRANDSON,                    // 孙子
    REL_GRANDDAUGHTER,               // 孙女
    REL_GREAT_GRANDSON,              // 曾孙子
    REL_GREAT_GRANDDAUGHTER,         // 曾孙女
    REL_COUSIN_SON,                  // 堂侄/表侄
    REL_COUSIN_DAUGHTER,             // 堂侄女/表侄女
    REL_GRAND_NEPHEW,                // 孙侄（兄弟的孙子）
    REL_GRAND_NIECE,                 // 孙侄女（兄弟的孙女）
    REL_NULL                         // 未知
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
}relation_call_t;

typedef struct relation_cal_s
{
    struct 
    {
        lv_obj_t *title;
        lv_obj_t *note;
        lv_obj_t *btnm;
        lv_obj_t *screen;
    }ui;
    
    relation_type_t rel_list[MAX_REL_LEN];
    int rel_count;
    relation_cal_status_t status;

}relation_cal_t;


typedef struct relation_transformation_s {
    relation_type_t from;
    relation_type_t to;
    relation_type_t result;
} relation_transformation_t;

void relation_cal_app_create(void);




#endif /* __RRLATION_CAL_H__ */

