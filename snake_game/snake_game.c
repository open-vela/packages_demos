#include "snake_game.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static snake_game_t g_game;
static int last_score = 0;          // Track the previous score
static int no_score_frames = 0;     // Count frames without score increase

// Forward declarations
static void snake_game_draw(void);
static void snake_game_timer_cb(lv_timer_t* timer);
static void snake_game_key_event_cb(lv_event_t* e);
static void generate_food(void);
static void auto_move(void);
static void start_button_event_cb(lv_event_t * e);
static void restart_button_event_cb(lv_event_t * e);

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
    
    // Create start button
    g_game.ui.start_btn = lv_btn_create(root);
    lv_obj_set_size(g_game.ui.start_btn, 200, 60);
    lv_obj_center(g_game.ui.start_btn);
    lv_obj_add_event_cb(g_game.ui.start_btn, start_button_event_cb, LV_EVENT_CLICKED, NULL);
    
    // Add label to the start button
    lv_obj_t * start_label = lv_label_create(g_game.ui.start_btn);
    lv_label_set_text(start_label, "START");
    lv_obj_set_style_text_font(start_label, &lv_font_montserrat_24, 0);
    lv_obj_center(start_label);
    
    // Create restart button (initially hidden)
    g_game.ui.restart_btn = lv_btn_create(root);
    lv_obj_set_size(g_game.ui.restart_btn, 200, 60);
    lv_obj_center(g_game.ui.restart_btn);
    lv_obj_add_event_cb(g_game.ui.restart_btn, restart_button_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(g_game.ui.restart_btn, LV_OBJ_FLAG_HIDDEN);  // Initially hidden
    
    // Add label to the restart button
    lv_obj_t * restart_label = lv_label_create(g_game.ui.restart_btn);
    lv_label_set_text(restart_label, "RESTART");
    lv_obj_set_style_text_font(restart_label, &lv_font_montserrat_24, 0);
    lv_obj_center(restart_label);
    
    // Initialize game in paused state
    g_game.status = GAME_STATUS_INIT;
    g_game.game_timer = lv_timer_create(snake_game_timer_cb, 50, NULL);
    lv_timer_pause(g_game.game_timer);
    
    // Set initial labels
    lv_label_set_text(g_game.ui.score_label, "Score: 0");
    lv_label_set_text(g_game.ui.status_label, "Ready");
    
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
    
    // Reset infinite loop detection variables
    last_score = 0;
    no_score_frames = 0;
    
    // Generate first food
    generate_food();
    
    // Update display
    lv_label_set_text(g_game.ui.score_label, "Score: 0");
}

static void snake_game_timer_cb(lv_timer_t* timer)
{
    if (g_game.status != GAME_STATUS_RUNNING) {
        return;
    }
    
    // Check for infinite loop condition
    if (g_game.score == last_score) {
        no_score_frames++;
        // If no score increase for 300 frames (about 15 seconds), consider it stuck
        if (no_score_frames > 300) {
            g_game.status = GAME_STATUS_OVER;
            lv_label_set_text(g_game.ui.status_label, "STUCK! GAME OVER");
            lv_timer_pause(g_game.game_timer);
            // Show restart button
            lv_obj_clear_flag(g_game.ui.restart_btn, LV_OBJ_FLAG_HIDDEN);
            lv_obj_t* restart_label = lv_obj_get_child(g_game.ui.restart_btn, 0);
            lv_label_set_text(restart_label, "RESTART");
            return;
        }
    } else {
        // Reset counter when score changes
        last_score = g_game.score;
        no_score_frames = 0;
    }
    
    // Check score for victory
    if (g_game.score >= 1000) {
        lv_label_set_text(g_game.ui.status_label, "Victory!");
        g_game.status = GAME_STATUS_OVER;
        lv_timer_pause(g_game.game_timer);
        // Show victory button
        lv_obj_clear_flag(g_game.ui.restart_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_t* restart_label = lv_obj_get_child(g_game.ui.restart_btn, 0);
        lv_label_set_text(restart_label, "Victory!");
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
    
    // Handle wall collision - turn around instead of crossing
    bool wall_collision = false;
    if (g_game.snake_head->x < 0 || g_game.snake_head->x >= GAME_GRID_WIDTH ||
        g_game.snake_head->y < 0 || g_game.snake_head->y >= GAME_GRID_HEIGHT) {
        // Restore position
        g_game.snake_head->x = old_x;
        g_game.snake_head->y = old_y;
        // Reverse direction
        switch (g_game.direction) {
            case DIR_UP:    g_game.direction = DIR_DOWN; break;
            case DIR_DOWN:  g_game.direction = DIR_UP; break;
            case DIR_LEFT:  g_game.direction = DIR_RIGHT; break;
            case DIR_RIGHT: g_game.direction = DIR_LEFT; break;
        }
        wall_collision = true;
        
        // Execute one step in the new direction immediately
        switch (g_game.direction) {
            case DIR_UP:    g_game.snake_head->y--; break;
            case DIR_DOWN:  g_game.snake_head->y++; break;
            case DIR_LEFT:  g_game.snake_head->x--; break;
            case DIR_RIGHT: g_game.snake_head->x++; break;
        }
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
        // If hit self, game over
        g_game.snake_head->x = old_x;
        g_game.snake_head->y = old_y;
        g_game.status = GAME_STATUS_OVER;
        lv_label_set_text(g_game.ui.status_label, "GAME OVER");
        lv_timer_pause(g_game.game_timer);
        // Show restart button
        lv_obj_clear_flag(g_game.ui.restart_btn, LV_OBJ_FLAG_HIDDEN);
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
    
    if (wall_collision) {
        // Recalculate path after turning around
        auto_move();
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
    int attempts = 0;
    const int MAX_ATTEMPTS = 100;  // Maximum attempts to find optimal position
    const int MIN_BORDER_DISTANCE = 2;  // Minimum distance from borders
    
    do {
        valid = true;
        if (attempts < MAX_ATTEMPTS) {
            // Try to generate food away from borders first
            g_game.food_x = MIN_BORDER_DISTANCE + (rand() % (GAME_GRID_WIDTH - 2 * MIN_BORDER_DISTANCE));
            g_game.food_y = MIN_BORDER_DISTANCE + (rand() % (GAME_GRID_HEIGHT - 2 * MIN_BORDER_DISTANCE));
        } else {
            // If too many attempts, allow food anywhere as fallback
            g_game.food_x = rand() % GAME_GRID_WIDTH;
            g_game.food_y = rand() % GAME_GRID_HEIGHT;
        }
        
        // Check if food is generated on snake body
        snake_node_t* current = g_game.snake_head;
        while (current != NULL) {
            if (current->x == g_game.food_x && current->y == g_game.food_y) {
                valid = false;
                break;
            }
            current = current->next;
        }
        
        attempts++;
        
        // If snake is too long and we can't find space in preferred area,
        // allow food generation anywhere after MAX_ATTEMPTS tries
        if (attempts > MAX_ATTEMPTS && !valid) {
            g_game.food_x = rand() % GAME_GRID_WIDTH;
            g_game.food_y = rand() % GAME_GRID_HEIGHT;
            valid = true;  // Force exit but keep checking snake collision
        }
    } while (!valid);
}

static bool is_position_safe(int x, int y) {
    // Check if position is out of bounds
    if (x < 0 || x >= GAME_GRID_WIDTH || y < 0 || y >= GAME_GRID_HEIGHT) {
        return false;
    }
    
    // Check if position collides with snake body
    snake_node_t* check_node = g_game.snake_head->next;
    while (check_node != NULL) {
        if (check_node->x == x && check_node->y == y) {
            return false;
        }
        check_node = check_node->next;
    }
    
    return true;
}

static void auto_move(void)
{
    int current_x = g_game.snake_head->x;
    int current_y = g_game.snake_head->y;
    
    // Define possible movement directions
    struct {
        direction_t dir;
        int dx;
        int dy;
        bool opposite;
    } directions[] = {
        {DIR_UP, 0, -1, g_game.direction == DIR_DOWN},
        {DIR_DOWN, 0, 1, g_game.direction == DIR_UP},
        {DIR_LEFT, -1, 0, g_game.direction == DIR_RIGHT},
        {DIR_RIGHT, 1, 0, g_game.direction == DIR_LEFT}
    };
    
    // Find best movement direction
    direction_t best_dir = g_game.direction;
    bool found_safe_move = false;
    int best_score = -1;
    
    // Calculate Manhattan distance to food
    int food_distance = abs(g_game.food_x - current_x) + abs(g_game.food_y - current_y);
    
    // Evaluate each possible direction
    for (int i = 0; i < 4; i++) {
        if (directions[i].opposite) continue;  // Don't move in opposite direction
        
        int new_x = current_x + directions[i].dx;
        int new_y = current_y + directions[i].dy;
        
        if (!is_position_safe(new_x, new_y)) continue;
        
        // Calculate score for this direction
        int score = 0;
        
        // 1. Check safe spaces in 8 surrounding cells
        int safe_spaces = 0;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                if (is_position_safe(new_x + dx, new_y + dy)) {
                    safe_spaces++;
                }
            }
        }
        score += safe_spaces * 3;  // Increase weight of safe spaces
        
        // 2. Calculate new distance to food
        int new_food_distance = abs(g_game.food_x - new_x) + abs(g_game.food_y - new_y);
        
        // Add score if getting closer to food
        if (new_food_distance < food_distance) {
            score += 10;  // Increase weight of food direction
        }
        
        // 3. Look ahead two steps
        for (int j = 0; j < 4; j++) {
            if (directions[j].opposite) continue;
            int look_x = new_x + directions[j].dx;
            int look_y = new_y + directions[j].dy;
            if (is_position_safe(look_x, look_y)) {
                score += 2;  // Reward paths with more future options
            }
        }
        
        // 4. Avoid moving along walls
        if ((new_x == 0 || new_x == GAME_GRID_WIDTH - 1) &&
            (new_y > 0 && new_y < GAME_GRID_HEIGHT - 1)) {
            score -= 5;  // Penalty for moving along vertical walls
        }
        if ((new_y == 0 || new_y == GAME_GRID_HEIGHT - 1) &&
            (new_x > 0 && new_x < GAME_GRID_WIDTH - 1)) {
            score -= 5;  // Penalty for moving along horizontal walls
        }
        
        // 5. Prefer current direction only if other factors are equal
        if (directions[i].dir == g_game.direction) {
            score += 1;
        }
        
        // Update best direction
        if (score > best_score) {
            best_score = score;
            best_dir = directions[i].dir;
            found_safe_move = true;
        }
    }
    
    // Update direction if safe move found
    if (found_safe_move) {
        g_game.direction = best_dir;
    }
}

static void start_button_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // Hide the start button
        lv_obj_t * btn = lv_event_get_target(e);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
        
        // Start the game
        snake_game_init();
        g_game.status = GAME_STATUS_RUNNING;
        lv_label_set_text(g_game.ui.status_label, "Auto Running");
        lv_timer_resume(g_game.game_timer);
    }
}

static void restart_button_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // Hide the restart button
        lv_obj_t * btn = lv_event_get_target(e);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
        
        // Start new game
        snake_game_init();
        g_game.status = GAME_STATUS_RUNNING;
        lv_label_set_text(g_game.ui.status_label, "Auto Running");
        lv_timer_resume(g_game.game_timer);
    }
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