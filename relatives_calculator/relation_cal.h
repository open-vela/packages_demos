#ifndef __RRLATION_CAL_H__
#define __RRLATION_CAL_H__


/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

#define REL_NODE_SUCCESS  1
#define REL_NODE_ERROR 0

#define REL_CALING 0
#define REL_CAIL_COMPELETE 1

#define MAX_TREE_LEN 256

typedef enum gender_e {
    GENDER_MALE,
    GENDER_FEMALE,
    GENDER_UNKNOWN
}gender_t;

typedef enum relation_type_e{
    REL_FATHER,
    REL_MOTHER,
    REL_SON,
    REL_DAUGHTER,
    REL_WIFE,
    REL_HUSBAND,
    REL_ELDER_BROTHER,
    REL_YOUNGER_BROTHER, 
    REL_ELDER_SISTER,
    REL_YOUNGER_SISTER,
    REL_NULL
}relation_type_t;

typedef struct relation_node_s
{
    enum relation_type_e relation;
    struct relation_node_s *next;
}relation_node_t;


typedef struct btnm_relation_s
{
    const char *btnm_text;
    relation_type_t relation;
    gender_t gender;
}btnm_relation_t;


typedef struct relation_tree_s
{
    enum gender_e gender;
    const char *call;
    
    struct relation_tree_s *father;
    struct relation_tree_s *mother;
    struct relation_tree_s *wife;
    struct relation_tree_s *husband;
    struct relation_tree_s *son;
    struct relation_tree_s *daughter;
    struct relation_tree_s *elder_bro;
    struct relation_tree_s *younger_bro;
    struct relation_tree_s *elder_sis;
    struct relation_tree_s *younger_sis;
}relation_tree_t;

typedef struct relation_cal_s
{
    struct 
    {
        lv_obj_t *title;
        lv_obj_t *note;
        lv_obj_t *btnm;
        lv_obj_t *screen;
    }ui;
    
    struct relation_node_s *rel_list;
    relation_tree_t mine;

    uint8_t status;
}relation_cal_t;

void relation_cal_app_create(void);




#endif /* __RRLATION_CAL_H__ */

