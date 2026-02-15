#ifndef LVGL_APP_H
#define LVGL_APP_H

/*********************
 *      INCLUDES
 *********************/

#include "stdio.h"
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

#define RES_ROOT CONFIG_LVX_CHINESE_CHESS_DATA_ROOT "/res"
#define FONTS_ROOT RES_ROOT "/fonts"
#define ICONS_ROOT RES_ROOT "/icons"
#define redCamp 1   
#define blackCamp 0
#define alive 1 
#define death 0 
#define red_che_1 0
#define red_ma_1 1
#define red_xiang_1 2
#define red_shi_1 3
#define red_jiang 4
#define red_shi_2 5
#define red_xiang_2 6
#define red_ma_2 7
#define red_che_2 8
#define red_pao_1 9
#define red_pao_2 10
#define red_bing_1 11
#define red_bing_2 12
#define red_bing_3 13
#define red_bing_4 14
#define red_bing_5 15
#define black_che_1 16
#define black_ma_1 17
#define black_xiang_1 18
#define black_shi_1 19
#define black_jiang 20
#define black_shi_2 21
#define black_xiang_2 22
#define black_ma_2 23
#define black_che_2 24
#define black_pao_1 25
#define black_pao_2 26
#define black_bing_1 27
#define black_bing_2 28
#define black_bing_3 29
#define black_bing_4 30
#define black_bing_5 31
#define board_black -1

 /**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    char* name;
    int Xvalue;
    int Yvalue;
    int camp;   
    int (*judge)(int x1,int y1,int x2,int y2,int Camp);
    int statue;
}pieceStruct;

typedef struct {
    char* name;
    int Xvalue;
    int Yvalue;
    int camp;  
}pieceStruct1;

typedef struct{
    /* data */
    int last_index;  
    int new_index;    
    int last_x;
    int last_y;
    int new_x;
    int new_y;
    int last_camp;
    int new_camp;
}changestrcut;

void Chinese_Chess_app_create(void);

#endif
