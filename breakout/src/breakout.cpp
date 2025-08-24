/******************************************************************
 * breakout.cpp
 * Main game logic implementation file
 * Contains all method implementations for the Game class, as well as
 * the global game entry point ballgame_start().
 * The Game class manages the game state, object lifecycle, and core logic.
 ********************************************************************/

#include "breakout.h"
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <stdio.h> 

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
      m_resourceManager(new GameResourceManager()) 
{
    init();
}

/**
 * Destructor of the Game class
 * Responsible for safely cleaning up all dynamically allocated resources upon object destruction to prevent memory leaks.
 */
Game::~Game() {
    printf("[Game] Destructor called, starting resource cleanup.\n");
    
    // Clean up static resource cache
    GameResourceManager::cleanupCache();
    if (m_resourceManager) {
        m_resourceManager->stopAudio();
        delete m_resourceManager;
        m_resourceManager = nullptr;
    }
    printf("[Game] Stopped audio\n");
    
    // Delete all PowerUp
    for (PowerUp* pu : m_powerUps) {
        pu->remove();
        delete pu;
    }
    m_powerUps.clear();
    printf("[Game] Deleted all PowerUp\n");
    // Delete all balls
    for (Ball* ball : m_balls) {
        delete ball;  // free Ball itself
    }
    m_balls.clear();
    printf("[Game] Deleted all balls\n");
    // Delete the game timer
    if (m_game_timer) {
        lv_timer_del(m_game_timer);
        m_game_timer = nullptr;
    }
    printf("[Game] Deleted game timer\n");
    // Delete all bricks
    for (Brick* b : m_bricks) {
        if (b->getGUIObject()) {
            lv_obj_del(b->getGUIObject());  // delete LVGL object
        }
        delete b; 
    }
    m_bricks.clear();
    printf("[Game] Deleted bricks\n");

    // Delete the paddle
    delete m_paddle;
    printf("[Game] Deleted paddle\n");
    
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

    // Load level map and background
    char bgFile[32];
    char mapFile[32];
    snprintf(bgFile, sizeof(bgFile), "background_%d.png", m_resourceManager->getCurrentLevel());
    snprintf(mapFile, sizeof(mapFile), "level_%d.dat", m_resourceManager->getCurrentLevel());
    m_resourceManager->loadBackground(m_game_area, bgFile);
    m_resourceManager->loadMap(mapFile, this);

    // Create paddle
    m_paddle = new Paddle(m_game_area,
                          (GAME_AREA_WIDTH / 2.0f) - 50.0f,
                          GAME_AREA_HEIGHT - 70.0f,
                          80.0f, 80.0f, m_gameBounds);
    printf("[Game] Paddle created\n");

    // Create first ball
    Ball* newBall = new Ball(m_game_area, 16.0f, Ball::State::HELD);
    m_balls.push_back(newBall);


    printf("[Game] Ball created\n");

    // Create transparent touch layer to capture input
    lv_obj_t* touch_area = lv_obj_create(m_game_area);
    lv_obj_set_size(touch_area, GAME_AREA_WIDTH, GAME_AREA_HEIGHT);
    lv_obj_set_pos(touch_area, 0, 0);
    lv_obj_set_style_bg_opa(touch_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(touch_area, 0, 0);
    lv_obj_add_event_cb(touch_area, touch_area_event_cb, LV_EVENT_ALL, this);
    lv_obj_move_background(touch_area);
    
    // Create game loop timer
    m_game_timer = lv_timer_create(game_timer_cb, GAME_TICK_PERIOD, this);
    static uint8_t cbuf[LV_CANVAS_BUF_SIZE(GAME_AREA_WIDTH, GAME_AREA_HEIGHT, 32, LV_DRAW_BUF_STRIDE_ALIGN)];
    
    // Add a canvas where all the balls will be drawn, and update their positions together.
    m_canvas = lv_canvas_create(m_game_area);
    lv_canvas_set_buffer(m_canvas, cbuf, GAME_AREA_WIDTH, GAME_AREA_HEIGHT, LV_COLOR_FORMAT_ARGB8888);
    lv_obj_set_style_bg_opa(m_canvas, LV_OPA_TRANSP, 0);
    lv_obj_align(m_canvas, LV_ALIGN_CENTER, 0, 0);


    printf("[Game] Init End\n");
}

/**
 * Game main loop timer callback
 * Updates paddle, balls, collisions, and checks win/lose conditions
 * @param timer Pointer to the LVGL timer
 */
void Game::game_timer_cb(lv_timer_t* timer) {
    Game* game = static_cast<Game*>(timer->user_data);
    if (game->m_state != GameState::PLAYING) return;

    const float deltaTime = (float)GAME_TICK_PERIOD / 1000.0f;

    // --- Update paddle position ---
    game->m_paddle->update(deltaTime);

    // --- Iterate through all balls ---
    for (auto it = game->m_balls.begin(); it != game->m_balls.end();) {
        Ball* ball = *it;
        Vec2 ballPos = ball->getPosition();

        // Check if the ball has fallen below the bottom boundary
        if (ballPos.y - ball->getRadius() >= game->m_gameBounds.y + game->m_gameBounds.height) {
            ball->setState(Ball::State::STATIC);  
            ball->setVelocity({0.0f, 0.0f});       
        }

        if (ball->getState() == Ball::State::HELD) {
            ball->stickToPaddle(game->m_paddle->getBoundingBox());
            ++it;
            continue;
        }

        if (ball->getState() == Ball::State::STATIC) {
            delete ball;  
            it = game->m_balls.erase(it);
            continue;
        }

        // MOVING state: update ball physics
        ball->update(deltaTime);

        // Horizontal walls
        if (ballPos.x - ball->getRadius() < game->m_gameBounds.x) {
            ball->bounceX();
            ball->setPosition({game->m_gameBounds.x + ball->getRadius(), ballPos.y});
        } else if (ballPos.x + ball->getRadius() > game->m_gameBounds.x + game->m_gameBounds.width) {
            ball->bounceX();
            ball->setPosition({game->m_gameBounds.x + game->m_gameBounds.width - ball->getRadius(), ballPos.y});
        }

        // Top wall
        if (ballPos.y - ball->getRadius() < game->m_gameBounds.y) {
            ball->bounceY();
            ball->boostSpeed(4.0f);
            ball->setPosition({ballPos.x, game->m_gameBounds.y + ball->getRadius()});
        }

        // Ball-brick collision
        game->handleBallBrickCollision(ball);

        // Ball-paddle collision
        game->handleBallPaddleCollision(ball);

        ++it;
    }

    // --- Update all power-ups ---
    for (auto it = game->m_powerUps.begin(); it != game->m_powerUps.end();) {
        PowerUp* pu = *it;
        pu->update(deltaTime);

        if (game->checkPowerUpPaddleCollision(pu, game->m_paddle)) {
            game->activatePowerUp(pu->getType());
            pu->remove();
            delete pu;
            it = game->m_powerUps.erase(it);
            continue;
        }

        if (pu->getPosition().y > game->m_gameBounds.y + game->m_gameBounds.height) {
            pu->remove();
            delete pu;
            it = game->m_powerUps.erase(it);
            continue;
        }

        ++it;
    }

    // --- Check pass level ---
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

    // --- Check game over ---
    bool allStatic = true;
    for (Ball* ball : game->m_balls) {
        if (ball->getState() != Ball::State::STATIC) {
            allStatic = false;
            break;
        }
    }
    if (allStatic) {
        game->trigger_game_over();
    }

    // --- Render all balls on canvas ---
    lv_layer_t layer;
    lv_canvas_init_layer(game->m_canvas, &layer);

    // Eliminate the trailing effect of the balls.
    lv_canvas_fill_bg(game->m_canvas, lv_color_black(), LV_OPA_TRANSP);

    for (Ball* ball : game->m_balls) {
        Vec2 pos = ball->getPosition();
        auto ballImg = ball->get_ball_src();
        if (!ballImg) continue;

        lv_draw_image_dsc_t dsc;
        lv_draw_image_dsc_init(&dsc);
        dsc.src = ballImg;

        lv_area_t coords;
        coords.x1 = static_cast<int>(pos.x - ballImg->header.w / 2);
        coords.y1 = static_cast<int>(pos.y - ballImg->header.h / 2);
        coords.x2 = coords.x1 + ballImg->header.w - 1;
        coords.y2 = coords.y1 + ballImg->header.h - 1;

        lv_draw_image(&layer, &dsc, &coords);
        
    }

    lv_canvas_finish_layer(game->m_canvas, &layer);
    lv_obj_invalidate(game->m_canvas);

}


Ball* Game::getMainBall() {
    Ball* mainBall = nullptr;
    float minY = std::numeric_limits<float>::max();

    for (Ball* ball : m_balls) {
        if (ball->getState() == Ball::State::MOVING || ball->getState() == Ball::State::HELD) {
            float y = ball->getPosition().y;
            if (y < minY) {
                minY = y;
                mainBall = ball;
            }
        }
    }
    return mainBall;
}
/**
 * Collision detection helper - Ball vs Paddle
 */
bool Game::checkBallPaddleCollision(const Ball* ball, const Paddle* paddle) {
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
 * Collision detection helper - Ball vs Brick
 */
bool Game::checkBallBrickCollision(const Ball* ball, const Brick* brick) {
    Vec2 ballPos = ball->getPosition();
    float ballRadius = ball->getRadius();
    Rect brickRect = brick->getBoundingBox();
    float closestX = std::max(brickRect.x, std::min(ballPos.x, brickRect.x + brickRect.width));
    float closestY = std::max(brickRect.y, std::min(ballPos.y, brickRect.y + brickRect.height));
    float distanceX = ballPos.x - closestX;
    float distanceY = ballPos.y - closestY;
    return (distanceX * distanceX) + (distanceY * distanceY) < (ballRadius * ballRadius);
}

bool Game::checkPowerUpPaddleCollision(const PowerUp* pu, const Paddle* paddle) {
    Vec2 pos = pu->getPosition();
    Rect paddleRect = paddle->getBoundingBox();
    return (pos.x >= paddleRect.x && pos.x <= paddleRect.x + paddleRect.width &&
            pos.y >= paddleRect.y && pos.y <= paddleRect.y + paddleRect.height);
}
/**
 * Handle collision between a ball and all bricks
 */
void Game::handleBallBrickCollision(Ball* ball) {
    Vec2 ballPos = ball->getPosition();

    for (Brick* brick : m_bricks) {
        if (!brick->isActive()) continue;
        if (!checkBallBrickCollision(ball, brick)) continue;

        Rect brickRect = brick->getBoundingBox();
        Vec2 brickCenter = {brickRect.x + brickRect.width / 2.0f,
                            brickRect.y + brickRect.height / 2.0f};
        Vec2 offset = {ballPos.x - brickCenter.x,
                       ballPos.y - brickCenter.y};

        float overlapX = ball->getRadius() + (brickRect.width / 2.0f) - fabsf(offset.x);
        float overlapY = ball->getRadius() + (brickRect.height / 2.0f) - fabsf(offset.y);

        if (brick->getHP() == -1) {
            // Indestructible brick
            if (overlapX < overlapY) {
                ball->bounceX();
                ballPos.x = offset.x > 0 ? brickRect.x + brickRect.width + ball->getRadius()
                                         : brickRect.x - ball->getRadius();
            } else {
                ball->bounceY();
                ballPos.y = offset.y > 0 ? brickRect.y + brickRect.height + ball->getRadius()
                                         : brickRect.y - ball->getRadius();
            }
            ball->setPosition(ballPos);
            ball->boostSpeed(4.0f);
            break;
        }
        int hpBefore = brick->getHP();
        // Normal brick: damage and bounce
        m_resourceManager->playAudio(hpBefore);
        brick->onHit();
        int hpReduced = hpBefore - brick->getHP();
        m_totalHpReduced += hpReduced; // accumulate total HP reduced

        if (overlapX < overlapY) {
            ball->bounceX();
            ballPos.x = offset.x > 0 ? brickRect.x + brickRect.width + ball->getRadius()
                                     : brickRect.x - ball->getRadius();
        } else {
            ball->bounceY();
            ballPos.y = offset.y > 0 ? brickRect.y + brickRect.height + ball->getRadius()
                                     : brickRect.y - ball->getRadius();
        }
        ball->setPosition(ballPos);
        ball->boostSpeed(4.0f);

        // Only drop a power-up every 5 brick hits
        if (!brick->isActive()) {
            if (m_totalHpReduced >= 5) {
                m_totalHpReduced = 0;

                // >>> Limit: max 3 power-ups on screen
                if (m_powerUps.size() < 3) {
                    // Randomly choose a power-up type
                    int randVal = rand() % 3;
                    PowerUp::Type type;
                    int activeBricks = 0;
                    for (Brick* b : m_bricks) {
                        if (b->isActive() && b->getHP() != -1) {
                            activeBricks++;
                            if (activeBricks > 15) {
                                break; 
                            }
                        }
                    }
                    // When the number of bricks is less than 15, only TROPHY will drop.
                    if (activeBricks < 15) {
                        type = PowerUp::Type::TROPHY;
                    } else {
                        if (randVal == 0) type = PowerUp::Type::SPLIT_BALL;
                        else if (randVal == 1){
                            if(m_paddle->getWidth()/80 >= 5){
                                type = PowerUp::Type::SPLIT_BALL;
                            }
                            else{
                                type = PowerUp::Type::EXTRA_PADDLE;
                            }
                        }
                        else{
                            if(m_paddle->getWidth()/80 >= 3){
                                type = PowerUp::Type::UP_BALLS;
                            }
                            else{
                                type = PowerUp::Type::EXTRA_PADDLE;
                            }
                        }
                    }
                    // Drop position: center-top of the destroyed brick
                    Vec2 dropPos = { brickRect.x + brickRect.width / 2.0f, brickRect.y };
                    PowerUp* pu = new PowerUp(m_game_area, type, dropPos);
                    m_powerUps.push_back(pu);

                    //printf("[Game] Power-up dropped after 3 HP reduced, current count = %zu\n", m_powerUps.size());
                } else {
                    //printf("[Game] Power-up limit reached (3), no new drop.\n");
                }
            }
        }
        break;
    }
}

/**
 * Handle collision between a ball and the paddle
 */
void Game::handleBallPaddleCollision(Ball* ball) {
    if (!ball || !m_paddle) return;

    Vec2 ballPos = ball->getPosition();

    if (checkBallPaddleCollision(ball, m_paddle)) {
        Rect paddleRect = m_paddle->getBoundingBox();
        float paddleCenter = paddleRect.x + paddleRect.width / 2.0f;

        // Compute hit factor (-1 left, +1 right)
        float hitFactor = (ballPos.x - paddleCenter) / (paddleRect.width / 2.0f);
        const float maxAngleFactor = 350.0f;

        Vec2 newVel = ball->getVelocity();
        newVel.x = hitFactor * maxAngleFactor;

        // Always bounce upward
        if (newVel.y > 0) newVel.y = -newVel.y;

        ball->setVelocity(newVel);
        ball->boostSpeed(150.0f);

        // Correct position to stay above paddle
        Vec2 correctedPos = ballPos;
        correctedPos.y = paddleRect.y - ball->getRadius() - 1.0f;
        ball->setPosition(correctedPos);
    }
}

Ball* Game::getFreeBall(float radius) {
    //printf("m_balls.size():%d",m_balls.size());
    //if (m_balls.size() > 10) {
    //   return nullptr; 
    //}
    // create a new one and add to m_balls
    Ball* newBall = new Ball(m_game_area, radius, Ball::State::HELD);
    m_balls.push_back(newBall);
    return newBall;
}

/**
 * @brief Activate a power-up effect in the game.
 * @param type Type of the power-up to activate
 */
void Game::activatePowerUp(PowerUp::Type type) {
    switch (type) {
        case PowerUp::Type::SPLIT_BALL: {
            Ball* original = getMainBall();
            if (original) {
                Vec2 pos = original->getPosition();
                float r = original->getRadius();
                // Get two free balls from the m_balls
                Ball* left = getFreeBall(r);
                if(left!=nullptr){ 
                left->setPosition(pos);
                left->setVelocity({-200.0f, -700.0f});
                left->setState(Ball::State::MOVING);
                }
                Ball* right = getFreeBall(r);
                if(right!=nullptr){     
                right->setPosition(pos);
                right->setVelocity({200.0f, -700.0f});
                right->setState(Ball::State::MOVING);
                }
                // Original ball remains in play
            }
            break;
        }
        case PowerUp::Type::EXTRA_PADDLE: {
            // Double the paddle width
            if (m_paddle) {
                if(m_paddle->getWidth()/80 <= 4)
                {
                    float newWidth = m_paddle->getWidth() + 80;
                    m_paddle->setWidth(newWidth); // Adjust LVGL object width
                }
            }
            break;
        }
        case PowerUp::Type::UP_BALLS: {
            // Determine number of new balls based on paddle width
            int numBalls = m_paddle->getWidth() / 80; // example: 80 width per ball
            if (numBalls < 1) numBalls = 1;

            float paddleX = m_paddle->getX();
            float paddleY = m_paddle->getY();
            float paddleWidth = m_paddle->getWidth();
            float ballRadius = 16.0f;

            // Horizontal spacing for evenly distributed balls
            float spacing = paddleWidth / (numBalls + 1);

            for (int i = 0; i < numBalls; ++i) {
                Ball* newBall = getFreeBall(ballRadius);
                if(newBall!=nullptr){
                float x = paddleX + spacing * (i + 1); // evenly spaced across paddle
                float y = paddleY - ballRadius;       // just above paddle
                newBall->setPosition({x, y});
                float vx = (rand() % 2 == 0 ? -1.0f : 1.0f) * (100.0f);
                float vy = -900.0f;
                newBall->setVelocity({vx, vy});
                newBall->setState(Ball::State::MOVING);
                }
            }
            break;
        }
        case PowerUp::Type::TROPHY: {
            // >>> Force clear all bricks
            for (Brick* brick : m_bricks) {
                if (brick->isActive() && brick->getHP() != -1) {
                    while (brick->isActive()) {
                        brick->onHit(); 
                    }
                }
            }
            break;
        }

    }
}

/**
 * Touch event callback
 * Controls paddle movement and ball launching
 */
void Game::touch_area_event_cb(lv_event_t* e) {
    Game* game = static_cast<Game*>(lv_event_get_user_data(e));
    if (game->m_state != GameState::PLAYING) return;
    
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
        // Drag: move paddle
        lv_point_t screen_point;
        lv_indev_get_point(lv_event_get_indev(e), &screen_point);
        lv_coord_t game_area_x = lv_obj_get_x(game->m_game_area);
        lv_coord_t local_x = screen_point.x - game_area_x;
        game->m_paddle->moveTo(local_x);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        // Release: stop paddle and launch any held balls
        game->m_paddle->stopMovement();
        for (Ball* ball : game->m_balls) {
            if (ball->getState() == Ball::State::HELD) {
                ball->launch();
            }
        }
    }
}

/**
 * Restart button callback
 */
void Game::restart_button_event_cb(lv_event_t* e) {
    Game* game = static_cast<Game*>(lv_event_get_user_data(e));
    if (!game) return;

    delete game;          
    ballgame_start();     
}
/**
 * Next level button callback
 */ 
void Game::next_level_button_event_cb(lv_event_t* e) {
    Game* game = static_cast<Game*>(lv_event_get_user_data(e));
    if (!game) return;

    if (game->m_resourceManager) {
        int level = game->m_resourceManager->getCurrentLevel();
        level++;
        if (level > 6) level = 1;
        game->m_resourceManager->setCurrentLevel(level);
    }

    delete game;          
    ballgame_start();    
}

/**
 * Trigger next level screen
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
 * Trigger game over screen
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
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    new Game(screen);   
}