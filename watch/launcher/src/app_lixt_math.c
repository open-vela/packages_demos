#include "../include/app_list_math.h"

const int screen_size = 455;
	
const int app_item_size = 120;
	
	
float global_transform_factor = 1.0f; // 定义并初始化
	
	
const transform_info transforms[5][3] = {
	
    // {{0.10, 0.00, 0.00}, {0.20, 0.00, 0.00}, {0.10, 0.00, 0.00}},
	
    // {{0.90, 0.00, 0.00}, {1.00, 0.00, 0.00}, {0.90, 0.00, 0.00}},
	
    // {{1.10, 0.00, 0.00}, {1.20, 0.00, 0.00}, {1.10, 0.00, 0.00}},
	
    // {{0.90, 0.00, 0.00}, {1.00, 0.00, 0.00}, {0.90, 0.00, 0.00}},
	
    // {{0.10, 0.00, 0.00}, {0.20, 0.00, 0.00}, {0.10, 0.00, 0.00}},
	
	
    {{+0.32, +0.08, +0.10}, {+0.34, +0.00, +0.05}, {+0.32, -0.08, +0.10}},
	
    {{+0.75, +0.01, +0.03}, {+1.00, +0.00, -0.02}, {+0.75, -0.01, +0.03}},
	
    {{+1.00, -0.04, +0.00}, {+1.20, +0.00, +0.00}, {+1.00, +0.04, +0.00}},
	
    {{+0.75, +0.01, -0.03}, {+1.00, +0.00, +0.02}, {+0.75, -0.01, -0.03}},
	
    {{+0.32, +0.08, -0.10}, {+0.34, +0.00, -0.05}, {+0.32, -0.08, -0.10}},
	
};
	
	// 一行的图标各自的中心点
static void init_row_data(grid_item_info *row_info, int row_number)
	
{
	
    float offset_x = screen_size / 2.0f - app_item_size;
	
    float center_y = row_number * app_item_size;
	
	
    row_info[0] = (grid_item_info){
	
        .size = app_item_size,
	
        .center_x = offset_x,
	
        .center_y = center_y};
	
	
    row_info[1] = (grid_item_info){
	
        .size = app_item_size,
	
        .center_x = screen_size / 2.0f,
	
        .center_y = center_y};
	
	
    row_info[2] = (grid_item_info){
	
        .size = app_item_size,
	
        .center_x = screen_size - offset_x,
	
        .center_y = center_y};
	
}
	
	// 单个cell信息
void init_grid_item(grid_item_info *item_info, int index)
	
{
	
    int row_number = index / 3;
	
    float offset_x = screen_size / 2.0f - app_item_size;
	
    float center_y = row_number * app_item_size;
	
	
    item_info->center_y = center_y;
	
    item_info->size = app_item_size;
	
	
    switch (index % 3)
	
    {
	
    case 0:
	
        item_info->center_x = offset_x;
	
        break;
	
    case 1:
	
        item_info->center_x = screen_size / 2.0f;
	
        break;
	
    case 2:
	
        item_info->center_x = screen_size - offset_x;
	
        break;
	
    }
	
}
	
	// 初始化所有行信息
grid_item_info **build_gird_items(int rows)
	
{
	
    grid_item_info **items = (grid_item_info **)malloc(rows * sizeof(grid_item_info *));
	
	
    for (int i = 0; i < rows; i++)
	
    {
	
        items[i] = (grid_item_info *)malloc(3 * sizeof(grid_item_info));
	
        init_row_data(items[i], i);
	
    }
	
	
    return items;
	
}
	
	// 插值计算（线性），平滑过渡
static void transform_row(grid_item_info *row, float transform_factor)
	
{
	
    int transform_index = (int)transform_factor;
	
    float fraction = transform_factor - transform_index;
	
	
    for (int i = 0; i < 3; i++)
	
    {
	
        float scale = transforms[transform_index][i].scale;
	
        float offset_x = transforms[transform_index][i].offset_x;
	
        float offset_y = transforms[transform_index][i].offset_y;
	
	
        if (fraction != 0.0f)
	
        {
	
            float offset_x_outer = (i == 0) ? 0.08f : (i == 2 ? -0.08f : 0.0f);
	
            transform_info current = (transform_index < 5) ? transforms[transform_index][i] : (transform_info){0, offset_x_outer, -0.21f};
	
            transform_info previous = (transform_index > 0) ? transforms[transform_index - 1][i] : (transform_info){0, offset_x_outer, +0.21f};
	
	
            // 线性插值
	
            scale = previous.scale * fraction + current.scale * (1.0f - fraction);
	
            offset_x = previous.offset_x * fraction + current.offset_x * (1.0f - fraction);
	
            offset_y = previous.offset_y * fraction + current.offset_y * (1.0f - fraction);
	
        }
	
	
        row[i].size = row[i].size * (1.0f - (1.0f - scale) * global_transform_factor);
	
        row[i].center_x = row[i].center_x + (offset_x * global_transform_factor) * screen_size;
	
        row[i].center_y = row[i].center_y + (offset_y * global_transform_factor) * screen_size;
	
    }
	
}
	
	// grid表格平滑过渡
void transform_grid(grid_item_info **items, int rows, float scroll_row_number)
	
{
	
    float start_pos = scroll_row_number;
	
    int start_index = (int)start_pos;
	
    float fraction = start_pos - start_index;
	
	
    if (fraction < 0.0f)
    {

        start_index--;
	
        fraction += 1.0f;
    }
	
	
    // 重置items到初始状态
	
    for (int i = 0; i < rows; i++)
	
    {
	
        init_row_data(items[i], i);
	
    }
	
	
    // 更新items数据
	
    for (int i = 0; i < 5 + 1; i++)
	
    {
	
        int base_index = i + start_index;
	
	
        if (base_index >= 0 && base_index < rows)
	
        {
	
            transform_row(items[base_index], i + fraction);
	
        }
	
    }
	
}
	
	
void print_grid_items(grid_item_info **items, int rows)
	
{
	
    printf("\nGrid Items Content:\n");
	
    printf("Row   Left(size,x,y)   Middle(size,x,y)   Right(size,x,y)\n");
	
    printf("--------------------------------------------------------\n");
	
	
    for (int i = 0; i < rows; i++)
	
    {
	
        printf("[%2d]  (%4.1f,%5.1f,%5.1f)  (%4.1f,%5.1f,%5.1f)  (%4.1f,%5.1f,%5.1f)\n",
	
               i,
	
               items[i][0].size, items[i][0].center_x, items[i][0].center_y,
	
               items[i][1].size, items[i][1].center_x, items[i][1].center_y,
	
               items[i][2].size, items[i][2].center_x, items[i][2].center_y);
	
    }
	
    printf("--------------------------------------------------------\n\n");
	
}
	
	
void print_grid_item_array(const char *name, grid_item_info *items, int size)
	
{
	
    printf("\n%s Content:\n", name);
	
    printf("Index  Size    X      Y\n");
	
    printf("------------------------\n");
	
	
    for (int i = 0; i < size; i++)
	
    {
	
        printf("[%2d]  %5.1f  %5.1f  %5.1f\n",
	
               i,
	
               items[i].size,
	
               items[i].center_x,
	
               items[i].center_y);
	
    }
	
    printf("------------------------\n\n");
	
}