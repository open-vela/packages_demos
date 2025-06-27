#ifndef __SNAKE_GAME_H__
#define __SNAKE_GAME_H__

#include <lvgl/lvgl.h>

// Game grid size
#define GAME_GRID_SIZE    20      // Size of each grid cell (pixels)
#define GAME_GRID_WIDTH   60      // Game area width (cells)
#define GAME_GRID_HEIGHT  35      // Game area height (cells)

// Game status
typedef enum {
    GAME_STATUS_INIT,
    GAME_STATUS_RUNNING,
    GAME_STATUS_PAUSED,
    GAME_STATUS_OVER
} game_status_t;

// Movement direction
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} direction_t;

// Snake node structure
typedef struct snake_node {
    int x;
    int y;
    struct snake_node* next;
} snake_node_t;

// UI structure
typedef struct {
    lv_obj_t *canvas;         // Game canvas
    lv_obj_t *score_label;    // Score display
    lv_obj_t *status_label;   // Status display
    lv_obj_t *start_btn;      // Start button
    lv_obj_t *restart_btn;    // Restart button
} snake_game_ui_t;

// Font structure
typedef struct {
    lv_font_t *font;          // Game font
} snake_game_fonts_t;

// Game state structure
typedef struct {
    snake_game_ui_t ui;           // UI elements
    snake_game_fonts_t fonts;     // Font resources
    snake_node_t* snake_head;     // Snake head pointer
    direction_t direction;        // Current movement direction
    int food_x;                  // Food X coordinate
    int food_y;                  // Food Y coordinate
    int score;                   // Current score
    game_status_t status;        // Game status
    lv_timer_t* game_timer;      // Game timer
} snake_game_t;

// Function declarations
void snake_game_create(void);
void snake_game_init(void);
void snake_game_deinit(void);

#endif /* __SNAKE_GAME_H__ */ 