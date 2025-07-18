#ifndef BREAKOUT_H
#define BREAKOUT_H

#pragma once

#include "breakout_types.h"
#include <vector>    // Include C++ vector container to manage list of bricks
#include <string>    // Include C++ string class to handle file paths

class Paddle;
class Ball;
class Brick;
class GameResourceManager;

// Define core game states: playing or game over
enum class GameState { PLAYING, GAME_OVER };

// --- Main Game class definition ---
/**
 * Main Game class (Game)
 * Encapsulates all game states, objects, and core logic.
 * Creates and initializes a game session in the constructor,
 * and automatically cleans up all resources in the destructor.
 */
class Game {
public:
    // --- Constructor and Destructor ---
    Game(lv_obj_t* parent); // Constructor to create a game instance
    ~Game();                // Destructor to destroy game instance and release all resources

    // Exposed for external modules (e.g., GameResourceManager) to access and modify directly during map loading
    lv_obj_t* m_game_area;        // Pointer to the UI container for the game area
    std::vector<Brick*> m_bricks; // Dynamic array holding all brick objects

private:
    // State and core UI pointers
    lv_obj_t* m_parent_screen;    // Pointer to the parent screen where the game resides
    GameState m_state;            // Current game state (PLAYING or GAME_OVER)
    lv_timer_t* m_game_timer;     // Game main loop timer

    // Game area and boundaries
    Rect m_gameBounds;            // Logical bounds of the game area

    // Game logic objects
    Paddle* m_paddle;             // Paddle object
    Ball* m_ball;                 // Ball object
    GameResourceManager* m_resourceManager; // Resource manager object

private:
    // --- Internal helper functions ---
    void init();                                               // Initialize the game
    void trigger_game_over();                                  // Handle game over state
    bool checkCollision(const Ball* ball, const Brick* brick);   // Collision detection (ball-brick)
    bool checkCollision(const Ball* ball, const Paddle* paddle); // Collision detection (ball-paddle)
    void handleBallBrickCollision(); 
    void handleBallPaddleCollision();

    // --- Static callback functions ---
    static void game_timer_cb(lv_timer_t* timer);              // Main loop timer callback
    static void touch_area_event_cb(lv_event_t* e);            // Touch input event callback
    static void restart_button_event_cb(lv_event_t* e);        // Restart button click callback
};

// --- Global start function prototype ---
/**
 * Global game start/restart function interface
 * The only external entry point to start or restart the entire game.
 */
void ballgame_start();

#endif // BREAKOUT_H