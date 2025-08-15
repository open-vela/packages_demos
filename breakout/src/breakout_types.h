#pragma once 

#include "lvgl.h"      

// 2D vector struct, represents position and velocity
struct Vec2 { float x = 0.0f; float y = 0.0f; };

// Rectangle struct, represents position, size, and bounds
struct Rect { float x = 0.0f; float y = 0.0f; float width = 0.0f; float height = 0.0f; };

// Define global constants for game screen size and timer period
static const int GAME_AREA_WIDTH = 1200;
static const int GAME_AREA_HEIGHT = 760;
static const int GAME_TICK_PERIOD = 20; // Game logic update period (milliseconds)