/******************************************************************
 * breakout.cpp
 * Main game logic implementation file
 * Contains all method implementations for the Game class, as well as
 * the global game entry point ballgame_start().
 * The Game class manages the game state, object lifecycle, and core logic.
 ********************************************************************/

#include "breakout.h"
#include "Paddle/Paddle.h"
#include "Ball/Ball.h"
#include "Brick/Brick.h" 
#include "GameResourceManager/GameResourceManager.h"
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <stdio.h> 

// Global pointer holding and managing the current unique game instance
Game* g_game_instance = nullptr;

// #################################################
// ## Implementation of Game class member functions
// #################################################

/**
 * Constructor of the Game class
 * @param parent The LVGL parent object on which the Game instance is created (usually the screen)
 */
Game::Game(lv_obj_t* parent)
    : m_game_area(nullptr),
      m_bricks(),
      m_parent_screen(parent),
      m_state(GameState::PLAYING),
      m_game_timer(nullptr),
      m_gameBounds(),
      m_paddle(nullptr),
      m_ball(nullptr),
      m_resourceManager(new GameResourceManager()) 
{
    printf("[Game] Constructor called\n");
    init();
    printf("[Game] Init finished\n");
}

/**
 * Destructor of the Game class
 * Responsible for safely cleaning up all dynamically allocated resources upon object destruction to prevent memory leaks.
 */
Game::~Game() {
    printf("[Game] Destructor called, starting resource cleanup.\n");
    
    // 1. First, clean up the static resource cache
    GameResourceManager::cleanupCache();
    if (m_resourceManager) {
        m_resourceManager->stopAudio();
        delete m_resourceManager;
        m_resourceManager = nullptr;
    }
    printf("[Game] Stopped audio\n");
    // 2. Delete the game timer
    if (m_game_timer) {
        lv_timer_del(m_game_timer);
        m_game_timer = nullptr;
    }

    // 3. Delete all brick objects
    for (Brick* brick : m_bricks) {
        delete brick;
    }
    m_bricks.clear();
    printf("[Game] Deleted bricks\n");
    // 4. Delete the paddle, ball, and resource manager instance
    delete m_paddle;
    delete m_ball;

    printf("[Game] Resource cleanup completed.\n");
}
/**
 * @brief Initialize all game elements, UI, and timers
 */
void Game::init() {
    printf("[Game] Init Start\n");
    // Create the UI object serving as game background and container
    m_game_area = lv_obj_create(m_parent_screen);
    lv_obj_set_size(m_game_area, GAME_AREA_WIDTH, GAME_AREA_HEIGHT);
    lv_obj_center(m_game_area);
    lv_obj_set_style_bg_color(m_game_area, lv_color_white(), 0);
    lv_obj_set_style_border_width(m_game_area, 0, 0);
    lv_obj_set_style_radius(m_game_area, 0, 0);
    lv_obj_set_style_pad_all(m_game_area, 0, 0);
    lv_obj_clear_flag(m_game_area, LV_OBJ_FLAG_SCROLLABLE);

    // Define game logic boundaries (relative to m_game_area)
    m_gameBounds = {0, 0, (float)GAME_AREA_WIDTH, (float)GAME_AREA_HEIGHT};

    // Load level map
    //m_resourceManager = new GameResourceManager();
    char bgFile[32];
    char mapFile[32];
    snprintf(bgFile, sizeof(bgFile), "background_%d.png", m_resourceManager->getCurrentLevel());
    snprintf(mapFile, sizeof(mapFile), "level_%d.dat", m_resourceManager->getCurrentLevel());
    m_resourceManager->loadBackground(m_game_area, bgFile);
    m_resourceManager->loadMap(mapFile, this);

    // Create game object instances
    m_paddle = new Paddle(m_game_area, (GAME_AREA_WIDTH / 2.0f) - 50.0f, GAME_AREA_HEIGHT - 70.0f, 80.0f, 80.0f, m_gameBounds);
    printf("[Game] Paddle created\n");
    m_ball = new Ball(m_game_area, 16.0f);
    printf("[Game] Ball created\n");

    // Create a transparent touch layer overlaying the game area to receive player input
    lv_obj_t* touch_area = lv_obj_create(m_game_area);
    lv_obj_set_size(touch_area, GAME_AREA_WIDTH, GAME_AREA_HEIGHT);
    lv_obj_set_pos(touch_area, 0, 0);
    lv_obj_set_style_bg_opa(touch_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(touch_area, 0, 0);
    lv_obj_add_event_cb(touch_area, touch_area_event_cb, LV_EVENT_ALL, this);
    lv_obj_move_background(touch_area); // Move touch layer to the back to avoid covering other objects
    
    // Create the main game loop timer
    m_game_timer = lv_timer_create(game_timer_cb, GAME_TICK_PERIOD, this);

    printf("[Game] Init End\n");
    
}

/**
 * Game main loop timer callback
 * @param timer Pointer to the LVGL timer
 */
void Game::game_timer_cb(lv_timer_t* timer) {
    // Retrieve the Game instance pointer from user data
    Game* game = static_cast<Game*>(timer->user_data);
    // State protection: do nothing if game is not in PLAYING state
    if (game->m_state != GameState::PLAYING) return;

    const float deltaTime = (float)GAME_TICK_PERIOD / 1000.0f;
    
    // Update paddle position (reacting to player input)
    game->m_paddle->update(deltaTime);

    // Handle ball states separately
    if (game->m_ball->getState() == Ball::State::HELD) {
        // HELD state: ball follows the paddle movement
        game->m_ball->stickToPaddle(game->m_paddle->getBoundingBox());
    } else {
        // MOVING state: update ball physics and perform collision detection
        game->m_ball->update(deltaTime);
        Vec2 ballPos = game->m_ball->getPosition();

        // 1. Boundary collision detection
        if (ballPos.x - game->m_ball->getRadius() < game->m_gameBounds.x || ballPos.x + game->m_ball->getRadius() > game->m_gameBounds.x + game->m_gameBounds.width) {
            game->m_ball->bounceX();
        }
        if (ballPos.y - game->m_ball->getRadius() < game->m_gameBounds.y) {
            game->m_ball->bounceY();
            game->m_ball->boostSpeed(4.0f);
        }
        if (ballPos.y + game->m_ball->getRadius() > game->m_gameBounds.y + game->m_gameBounds.height) {
            game->trigger_game_over();
            return;
        }

        // 2. Collision detection with bricks
        game->handleBallBrickCollision();

        // 3. Collision detection with paddle
        game->handleBallPaddleCollision();

        // 4. Check if the level is cleared
        bool allCleared = true;
        for (Brick* b : game->m_bricks) {
            if (b->isActive() && b->getHP() != -1) {
                allCleared = false;
                break;
            }
        }
        if (allCleared) {
            game->trigger_next_level();
        }
    }
}
/**
 * Handle collision between the ball and the bricks.
 * Checks each active brick for collision with the ball.
 * If a collision occurs, bounce the ball appropriately and apply damage or special logic.
 */
void Game::handleBallBrickCollision() {
    Vec2 ballPos = m_ball->getPosition();

    for (auto it = m_bricks.begin(); it != m_bricks.end(); ++it) {
        Brick* brick = *it;
        if (!brick->isActive()) continue;            // Skip inactive bricks
        if (!checkCollision(m_ball, brick)) continue; // Skip if no collision

        Rect brickRect = brick->getBoundingBox();
        Vec2 brickCenter = {
            brickRect.x + brickRect.width / 2.0f,
            brickRect.y + brickRect.height / 2.0f
        };
        Vec2 offset = {
            ballPos.x - brickCenter.x,
            ballPos.y - brickCenter.y
        };

        // Calculate overlap distances on X and Y axes
        float overlapX = m_ball->getRadius() + (brickRect.width / 2.0f) - fabsf(offset.x);
        float overlapY = m_ball->getRadius() + (brickRect.height / 2.0f) - fabsf(offset.y);

        // Special case: Wall bricks (hp == -1) only bounce ball, do not get destroyed
        if (brick->getHP() == -1) {
            if (overlapX < overlapY) {
                m_ball->bounceX(); // Bounce horizontally
                // Correct ball position to avoid sticking inside the brick
                ballPos.x = offset.x > 0
                    ? brickRect.x + brickRect.width + m_ball->getRadius()
                    : brickRect.x - m_ball->getRadius();
            } else {
                m_ball->bounceY(); // Bounce vertically
                ballPos.y = offset.y > 0
                    ? brickRect.y + brickRect.height + m_ball->getRadius()
                    : brickRect.y - m_ball->getRadius();
            }
            m_ball->setPosition(ballPos);
            m_ball->boostSpeed(4.0f);  // Slightly increase ball speed after bounce
            break;                     // Exit after first collision handled
        }
        m_resourceManager->playAudio(brick->getHP());
        // Normal brick logic: apply damage and bounce ball
        brick->onHit();  // Reduce brick HP or mark as destroyed

        if (overlapX < overlapY) {
            m_ball->bounceX();
            ballPos.x = offset.x > 0
                ? brickRect.x + brickRect.width + m_ball->getRadius()
                : brickRect.x - m_ball->getRadius();
        } else {
            m_ball->bounceY();
            ballPos.y = offset.y > 0
                ? brickRect.y + brickRect.height + m_ball->getRadius()
                : brickRect.y - m_ball->getRadius();
        }
        m_ball->setPosition(ballPos);
        m_ball->boostSpeed(4.0f);
        break;
    }
}




/**
 * Handle collision between the ball and the paddle.
 * When collision is detected, the ball bounces off the paddle with an angle
 * depending on the hit position on the paddle.
 */
void Game::handleBallPaddleCollision() {
    if (!m_ball || !m_paddle) return;

    Vec2 ballPos = m_ball->getPosition();

    if (checkCollision(m_ball, m_paddle)) {
        Rect paddleRect = m_paddle->getBoundingBox();
        float paddleCenter = paddleRect.x + paddleRect.width / 2.0f;

        // Calculate hit factor relative to paddle center (-1 to 1)
        float hitFactor = (ballPos.x - paddleCenter) / (paddleRect.width / 2.0f);
        const float maxAngleFactor = 350.0f;  // Max horizontal velocity factor

        Vec2 newVel = m_ball->getVelocity();
        newVel.x = hitFactor * maxAngleFactor;  // Modify horizontal velocity based on hit position

        // Ensure the ball always bounces upward
        if (newVel.y > 0) newVel.y = -newVel.y;

        m_ball->setVelocity(newVel);
        m_ball->boostSpeed(150.0f);
        // Correct ball position to prevent it sticking inside the paddle
        Vec2 correctedPos = ballPos;
        correctedPos.y = paddleRect.y - m_ball->getRadius() - 1.0f;
        m_ball->setPosition(correctedPos);
    }
}
/**
 * Touch event callback, handles player touch input (static member function)
 */
void Game::touch_area_event_cb(lv_event_t* e) {
    Game* game = static_cast<Game*>(lv_event_get_user_data(e));
    if (game->m_state != GameState::PLAYING) return;
    
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
        // On press or drag, move the paddle
        lv_point_t screen_point;
        lv_indev_get_point(lv_event_get_indev(e), &screen_point);
        lv_coord_t game_area_x = lv_obj_get_x(game->m_game_area);
        lv_coord_t local_x = screen_point.x - game_area_x;
        game->m_paddle->moveTo(local_x);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        // On release, stop paddle movement and launch the ball
        game->m_paddle->stopMovement();
        if (game->m_ball->getState() == Ball::State::HELD) {
            game->m_ball->launch();
        }
    }
}


/**
 * Collision detection helper - Ball and Paddle
 */
bool Game::checkCollision(const Ball* ball, const Paddle* paddle) {
    Vec2 ballPos = ball->getPosition();
    float ballRadius = ball->getRadius();
    Rect paddleRect = paddle->getBoundingBox();
    float closestX = std::max(paddleRect.x, std::min(ballPos.x, paddleRect.x + paddleRect.width));
    float closestY = std::max(paddleRect.y, std::min(ballPos.y, paddleRect.y + paddleRect.height));
    float distanceX = ballPos.x - closestX;
    float distanceY = ballPos.y - closestY;
    return (distanceX * distanceX) + (distanceY * distanceY) < (ballRadius * ballRadius);
}

/**
 * Collision detection helper - Ball and Brick
 */
bool Game::checkCollision(const Ball* ball, const Brick* brick) {
    Vec2 ballPos = ball->getPosition();
    float ballRadius = ball->getRadius();
    Rect brickRect = brick->getBoundingBox();
    float closestX = std::max(brickRect.x, std::min(ballPos.x, brickRect.x + brickRect.width));
    float closestY = std::max(brickRect.y, std::min(ballPos.y, brickRect.y + brickRect.height));
    float distanceX = ballPos.x - closestX;
    float distanceY = ballPos.y - closestY;
    return (distanceX * distanceX) + (distanceY * distanceY) < (ballRadius * ballRadius);
}
/**
 * "Restart" button click event callback
 */
void Game::restart_button_event_cb(lv_event_t* e) {
    if (g_game_instance != nullptr) {
        delete g_game_instance;
        g_game_instance = nullptr;
    }
    ballgame_start();
}

/**
 * "Next level" button event callback
 */ 
void Game::next_level_button_event_cb(lv_event_t* e) {
    Game* game = static_cast<Game*>(lv_event_get_user_data(e));

    if (game->m_resourceManager) {
        int level = game->m_resourceManager->getCurrentLevel();
        level++;
        if (level > 7) level = 1;
        game->m_resourceManager->setCurrentLevel(level);
    }

    if (g_game_instance != nullptr) {
        delete g_game_instance;
        g_game_instance = nullptr;
    }
    ballgame_start();
}

/**
 * "Next level" button
 */ 
void Game::trigger_next_level() {
    m_state = GameState::PAUSED;
    lv_timer_pause(m_game_timer);

    lv_obj_t* label = lv_label_create(m_parent_screen);
    lv_label_set_text_fmt(label, "LEVEL %d CLEARED!", m_resourceManager->getCurrentLevel());
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_center(label);

    lv_obj_t* next_btn = lv_button_create(m_parent_screen);
    lv_obj_align_to(next_btn, label, LV_ALIGN_OUT_BOTTOM_MID, -35, 20);
    lv_obj_t* btn_label = lv_label_create(next_btn);
    lv_label_set_text(btn_label, "Next Level");
    lv_obj_center(btn_label);

    lv_obj_add_event_cb(next_btn, next_level_button_event_cb, LV_EVENT_CLICKED, this);
}

/**
 * @brief Trigger the game over state and display UI
 */
void Game::trigger_game_over() {
    m_state = GameState::GAME_OVER;
    lv_obj_t* game_over_label = lv_label_create(m_parent_screen);
    lv_label_set_text(game_over_label, "GAME OVER!");
    lv_obj_set_style_text_color(game_over_label, lv_color_black(), 0);
    lv_obj_center(game_over_label);
    
    lv_obj_t* restart_btn = lv_button_create(m_parent_screen);
    lv_obj_align_to(restart_btn, game_over_label, LV_ALIGN_OUT_BOTTOM_MID, -23, 20);
    lv_obj_t* btn_label = lv_label_create(restart_btn);
    lv_label_set_text(btn_label, "Restart");
    lv_obj_center(btn_label);
    
    lv_obj_add_event_cb(restart_btn, restart_button_event_cb, LV_EVENT_CLICKED, this);

    // "Skip level" button
    lv_obj_t* skip_btn = lv_button_create(m_parent_screen);
    lv_obj_align_to(skip_btn, game_over_label, LV_ALIGN_OUT_BOTTOM_MID, -35, 60);
    //lv_obj_set_size(skip_btn, 100, 40);
    lv_obj_t* skip_btn_label = lv_label_create(skip_btn);
    lv_label_set_text(skip_btn_label, "SKIP LEVEL");
    lv_obj_center(skip_btn_label);
    lv_obj_add_event_cb(skip_btn, next_level_button_event_cb, LV_EVENT_CLICKED, this);
}

// #################################################
// ## Global startup function
// #################################################

/**
 * Global game start/restart function
 * Manages creation and destruction of the Game instance.
 */
void ballgame_start() {
    lv_obj_t* screen = lv_screen_active();
    // Clean all old LVGL objects from the screen
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    // Create a new game instance
    g_game_instance = new Game(screen);
}

