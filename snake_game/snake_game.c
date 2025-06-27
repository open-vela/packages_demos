#include "snake_game.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static snake_game_t g_game;

// Forward declarations
static void snake_game_draw(void);
static void snake_game_timer_cb(lv_timer_t* timer);
static void snake_game_key_event_cb(lv_event_t* e);
static void generate_food(void);
static void auto_move(void);
static int get_snake_length(void);

void snake_game_create(void)
{
    // Create main interface
    lv_obj_t* root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(root, GAME_GRID_SIZE * GAME_GRID_WIDTH, GAME_GRID_SIZE * GAME_GRID_HEIGHT + 40);
    lv_obj_center(root);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);  // Disable scrolling
    lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE);     // Enable click
    
    // Create game canvas
    g_game.ui.canvas = lv_canvas_create(root);
    lv_obj_align(g_game.ui.canvas, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_size(g_game.ui.canvas, GAME_GRID_SIZE * GAME_GRID_WIDTH, GAME_GRID_SIZE * GAME_GRID_HEIGHT);
    lv_obj_set_style_bg_color(g_game.ui.canvas, lv_color_hex(0x000000), 0);
    
    // Create score label
    g_game.ui.score_label = lv_label_create(root);
    lv_obj_align(g_game.ui.score_label, LV_ALIGN_BOTTOM_LEFT, 10, -5);
    lv_obj_set_style_text_color(g_game.ui.score_label, lv_color_white(), 0);
    
    // Create status label
    g_game.ui.status_label = lv_label_create(root);
    lv_obj_align(g_game.ui.status_label, LV_ALIGN_BOTTOM_RIGHT, -10, -5);
    lv_obj_set_style_text_color(g_game.ui.status_label, lv_color_white(), 0);
    
    // Initialize game
    snake_game_init();
    
    // Add keyboard event processing
    lv_obj_t* scr = lv_scr_act();
    lv_obj_add_event_cb(scr, snake_game_key_event_cb, LV_EVENT_KEY, NULL);
    
    // Set group and focus
    lv_group_t* g = lv_group_create();
    lv_group_add_obj(g, scr);
    lv_indev_t* kb_indev = lv_indev_get_next(NULL);
    while(kb_indev) {
        if(lv_indev_get_type(kb_indev) == LV_INDEV_TYPE_KEYPAD) {
            lv_indev_set_group(kb_indev, g);
            break;
        }
        kb_indev = lv_indev_get_next(kb_indev);
    }
    lv_group_focus_obj(scr);
}

void snake_game_init(void)
{
    // Initialize random number generator
    srand(time(NULL));
    
    // Initialize snake
    g_game.snake_head = malloc(sizeof(snake_node_t));
    g_game.snake_head->x = GAME_GRID_WIDTH / 2;
    g_game.snake_head->y = GAME_GRID_HEIGHT / 2;
    g_game.snake_head->next = NULL;
    
    // Initialize game state
    g_game.direction = DIR_RIGHT;
    g_game.score = 0;
    g_game.status = GAME_STATUS_RUNNING;  // Set to running state directly
    
    // Generate first food
    generate_food();
    
    // Update display
    lv_label_set_text(g_game.ui.score_label, "Score: 0");
    lv_label_set_text(g_game.ui.status_label, "Auto Running");
    
    // Create and start game timer
    g_game.game_timer = lv_timer_create(snake_game_timer_cb, 50, NULL);  // Faster speed
}

static void snake_game_timer_cb(lv_timer_t* timer)
{
    if (g_game.status != GAME_STATUS_RUNNING) {
        return;
    }
    
    // Check snake length
    if (get_snake_length() >= 1000) {
        lv_label_set_text(g_game.ui.status_label, "Target Achieved!");
        g_game.status = GAME_STATUS_OVER;
        lv_timer_pause(g_game.game_timer);
        return;
    }

    // Auto move
    auto_move();
    
    // Save old head position
    int old_x = g_game.snake_head->x;
    int old_y = g_game.snake_head->y;
    
    // Move snake head according to direction
    switch (g_game.direction) {
        case DIR_UP:    g_game.snake_head->y--; break;
        case DIR_DOWN:  g_game.snake_head->y++; break;
        case DIR_LEFT:  g_game.snake_head->x--; break;
        case DIR_RIGHT: g_game.snake_head->x++; break;
    }
    
    // Handle boundary crossing
    if (g_game.snake_head->x < 0) {
        g_game.snake_head->x = GAME_GRID_WIDTH - 1;
    } else if (g_game.snake_head->x >= GAME_GRID_WIDTH) {
        g_game.snake_head->x = 0;
    }
    if (g_game.snake_head->y < 0) {
        g_game.snake_head->y = GAME_GRID_HEIGHT - 1;
    } else if (g_game.snake_head->y >= GAME_GRID_HEIGHT) {
        g_game.snake_head->y = 0;
    }
    
    // Check self collision
    snake_node_t* check_node = g_game.snake_head->next;
    bool self_collision = false;
    while (check_node != NULL) {
        if (check_node->x == g_game.snake_head->x && check_node->y == g_game.snake_head->y) {
            self_collision = true;
            break;
        }
        check_node = check_node->next;
    }
    
    if (self_collision) {
        // If hit self, retreat and change direction
        g_game.snake_head->x = old_x;
        g_game.snake_head->y = old_y;
        // Choose random new direction
        g_game.direction = rand() % 4;
        return;
    }
    
    // Check if food is eaten
    if (g_game.snake_head->x == g_game.food_x && g_game.snake_head->y == g_game.food_y) {
        // Increase score
        g_game.score += 10;
        char score_text[32];
        sprintf(score_text, "Score: %d", g_game.score);
        lv_label_set_text(g_game.ui.score_label, score_text);
        
        // Generate new food
        generate_food();
        
        // Increase snake length
        snake_node_t* new_node = malloc(sizeof(snake_node_t));
        new_node->x = old_x;
        new_node->y = old_y;
        new_node->next = g_game.snake_head->next;
        g_game.snake_head->next = new_node;
    } else {
        // Move snake body
        snake_node_t* move_node = g_game.snake_head->next;
        while (move_node != NULL) {
            int temp_x = move_node->x;
            int temp_y = move_node->y;
            move_node->x = old_x;
            move_node->y = old_y;
            old_x = temp_x;
            old_y = temp_y;
            move_node = move_node->next;
        }
    }
    
    // Update display
    snake_game_draw();
}

static void snake_game_key_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        
        switch (key) {
            case LV_KEY_UP:
                if (g_game.direction != DIR_DOWN) {
                    g_game.direction = DIR_UP;
                }
                break;
            case LV_KEY_DOWN:
                if (g_game.direction != DIR_UP) {
                    g_game.direction = DIR_DOWN;
                }
                break;
            case LV_KEY_LEFT:
                if (g_game.direction != DIR_RIGHT) {
                    g_game.direction = DIR_LEFT;
                }
                break;
            case LV_KEY_RIGHT:
                if (g_game.direction != DIR_LEFT) {
                    g_game.direction = DIR_RIGHT;
                }
                break;
            case LV_KEY_ENTER:
                if (g_game.status == GAME_STATUS_INIT || g_game.status == GAME_STATUS_OVER) {
                    snake_game_init();
                    g_game.status = GAME_STATUS_RUNNING;
                    lv_label_set_text(g_game.ui.status_label, "In Game");
                    lv_timer_resume(g_game.game_timer);
                } else if (g_game.status == GAME_STATUS_RUNNING) {
                    g_game.status = GAME_STATUS_PAUSED;
                    lv_label_set_text(g_game.ui.status_label, "Paused");
                    lv_timer_pause(g_game.game_timer);
                } else if (g_game.status == GAME_STATUS_PAUSED) {
                    g_game.status = GAME_STATUS_RUNNING;
                    lv_label_set_text(g_game.ui.status_label, "In Game");
                    lv_timer_resume(g_game.game_timer);
                }
                break;
        }
    }
}

static void snake_game_draw(void)
{
    // Delete all objects on canvas
    lv_obj_clean(g_game.ui.canvas);
    
    // Reset canvas background color
    lv_obj_set_style_bg_color(g_game.ui.canvas, lv_color_hex(0x000000), 0);
    
    // Draw food
    lv_obj_t* food = lv_obj_create(g_game.ui.canvas);
    lv_obj_set_size(food, GAME_GRID_SIZE - 2, GAME_GRID_SIZE - 2);
    lv_obj_set_pos(food, g_game.food_x * GAME_GRID_SIZE + 1, g_game.food_y * GAME_GRID_SIZE + 1);
    lv_obj_set_style_bg_color(food, lv_color_hex(0xFF0000), 0);
    
    // Draw snake
    snake_node_t* current = g_game.snake_head;
    while (current != NULL) {
        lv_obj_t* node = lv_obj_create(g_game.ui.canvas);
        lv_obj_set_size(node, GAME_GRID_SIZE - 2, GAME_GRID_SIZE - 2);
        lv_obj_set_pos(node, current->x * GAME_GRID_SIZE + 1, current->y * GAME_GRID_SIZE + 1);
        if (current == g_game.snake_head) {
            // Different color for snake head
            lv_obj_set_style_bg_color(node, lv_color_hex(0x00FF00), 0);
        } else {
            // Darker color for snake body
            lv_obj_set_style_bg_color(node, lv_color_hex(0x008800), 0);
        }
        current = current->next;
    }
}

static void generate_food(void)
{
    bool valid;
    do {
        valid = true;
        g_game.food_x = rand() % GAME_GRID_WIDTH;
        g_game.food_y = rand() % GAME_GRID_HEIGHT;
        
        // Check if food is generated on snake body
        snake_node_t* current = g_game.snake_head;
        while (current != NULL) {
            if (current->x == g_game.food_x && current->y == g_game.food_y) {
                valid = false;
                break;
            }
            current = current->next;
        }
    } while (!valid);
}

static void auto_move(void)
{
    // Calculate direction to food (considering shortest path through boundaries)
    int dx = g_game.food_x - g_game.snake_head->x;
    int dy = g_game.food_y - g_game.snake_head->y;
    
    // Consider crossing boundary cases
    if (abs(dx) > GAME_GRID_WIDTH / 2) {
        if (dx > 0) {
            dx = dx - GAME_GRID_WIDTH;
        } else {
            dx = dx + GAME_GRID_WIDTH;
        }
    }
    if (abs(dy) > GAME_GRID_HEIGHT / 2) {
        if (dy > 0) {
            dy = dy - GAME_GRID_HEIGHT;
        } else {
            dy = dy + GAME_GRID_HEIGHT;
        }
    }
    
    // Choose direction closest to food
    if (abs(dx) > abs(dy)) {
        if (dx > 0 && g_game.direction != DIR_LEFT) {
            g_game.direction = DIR_RIGHT;
        } else if (dx < 0 && g_game.direction != DIR_RIGHT) {
            g_game.direction = DIR_LEFT;
        }
    } else {
        if (dy > 0 && g_game.direction != DIR_UP) {
            g_game.direction = DIR_DOWN;
        } else if (dy < 0 && g_game.direction != DIR_DOWN) {
            g_game.direction = DIR_UP;
        }
    }
}

static int get_snake_length(void)
{
    int length = 1;  // Count head
    snake_node_t* current = g_game.snake_head->next;
    while (current != NULL) {
        length++;
        current = current->next;
    }
    return length;
}

void snake_game_deinit(void)
{
    // Free snake body memory
    snake_node_t* current = g_game.snake_head;
    while (current != NULL) {
        snake_node_t* next = current->next;
        free(current);
        current = next;
    }
    
    // Delete timer
    if (g_game.game_timer) {
        lv_timer_del(g_game.game_timer);
        g_game.game_timer = NULL;
    }
} 