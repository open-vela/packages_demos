#ifndef BREAKOUT_H
#define BREAKOUT_H

#pragma once
#include "breakout_types.h"
#include <vector>    // C++ vector container to store dynamic lists (e.g., bricks)
#include <string>    // C++ string class to handle file paths and names
#include "Paddle/Paddle.h"
#include "Ball/Ball.h"
#include "Brick/Brick.h" 
#include "PowerUp/PowerUp.h" 
#include "GameResourceManager/GameResourceManager.h"
// --- Game State Enumeration ---
/**
 * @brief Core game states
 */
enum class GameState {
    PLAYING,    ///< Game is running
    PAUSED,
    GAME_OVER,  ///< Game has ended
};

// --- Main Game Class ---
/**
 * @class Game
 * @brief Encapsulates all game states, objects, and core logic.
 * 
 * Creates and initializes a game session in the constructor,
 * and automatically cleans up all resources in the destructor.
 */
class Game {
public:
    /**
     * @brief Construct a new Game instance.
     * @param parent Pointer to the parent LVGL object (screen/container).
     */
    Game(lv_obj_t* parent);

    /**
     * @brief Destroy the Game instance and release all resources.
     */
    ~Game();

    // --- Public Members ---
    /**
     * @brief Pointer to the UI container for the game area.
     * Exposed for access by GameResourceManager during map loading.
     */
    lv_obj_t* m_game_area;

    /**
     * @brief Dynamic array holding all brick objects.
     * Exposed for access by GameResourceManager during map loading.
     */
    std::vector<Brick*> m_bricks;

private:
    // --- Core State and UI ---
    lv_obj_t* m_parent_screen;    ///< Parent screen where the game resides
    GameState m_state;            ///< Current game state
    lv_timer_t* m_game_timer;     ///< Main game loop timer

    // --- Game Boundaries ---
    Rect m_gameBounds;            ///< Logical bounds of the game area

    // --- Game Objects ---
    Paddle* m_paddle;             ///< Paddle object
    GameResourceManager* m_resourceManager; ///< Resource manager
    int m_currentLevel;
    std::vector<PowerUp*> m_powerUps;
    int m_totalHpReduced = 0;
    std::vector<Ball*> m_balls;        // Active balls
    lv_obj_t* m_canvas;
    

private:
    // --- Internal Methods ---
    void init();                                               ///< Initialize game objects and state
    void trigger_game_over();                                  ///< Handle game over logic
    void trigger_next_level();

    bool checkBallBrickCollision(const Ball* ball, const Brick* brick); ///< Detect collision between ball and brick
    bool checkBallPaddleCollision(const Ball* ball, const Paddle* paddle); ///< Detect collision between ball and paddle
    bool checkPowerUpPaddleCollision(const PowerUp* pu, const Paddle* paddle);
    void handleBallBrickCollision(Ball* ball);    ///< Handle ball-brick collision events
    void handleBallPaddleCollision(Ball* ball);   ///< Handle ball-paddle collision events
    Ball* getMainBall();
    Ball* getFreeBall(float radius);
    void activatePowerUp(PowerUp::Type type);


    // --- Static Callbacks ---
    static void game_timer_cb(lv_timer_t* timer);              ///< Main game loop callback
    static void touch_area_event_cb(lv_event_t* e);            ///< Touch input callback
    static void restart_button_event_cb(lv_event_t* e);        ///< Restart button callback
    static void next_level_button_event_cb(lv_event_t* e);
};

// --- Global Function ---
/**
 * @brief Start or restart the ball game.
 * 
 * This is the only external entry point to start/restart the game.
 */
void ballgame_start();

#endif // BREAKOUT_H