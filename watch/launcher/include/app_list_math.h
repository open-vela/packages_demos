#ifndef __APP_LIST_MATH_H
	
#define __APP_LIST_MATH_H
	
	
#include <stdlib.h>
	
#include <malloc.h>
	
#include <stdio.h>
	
	
typedef struct {
	
    float size;
	
    float center_x;
	
    float center_y;
	
} grid_item_info;
	
	
typedef struct {
	
    float scale;
	
    float offset_x;
	
    float offset_y;
	
} transform_info;
	
	
// 根据滚动行数变换网格
	
void transform_grid(grid_item_info** items, int rows, float scroll_row_number);
	
	
// 构建网格项目数组
	
grid_item_info** build_gird_items(int rows);
	
	
// 初始化单个网格项目
	
void init_grid_item(grid_item_info* item_info, int index);
	
	
// 打印单个grid_item_info数组的内容
	
void print_grid_item_array(const char* name, grid_item_info* items, int size);
	
	
// 全局常量和变量声明
	
extern const int screen_size;
	
extern const int app_item_size;
	
extern const transform_info transforms[][3];
	
extern float global_transform_factor;
	
	
#endif // __APP_LIST_MATH_H