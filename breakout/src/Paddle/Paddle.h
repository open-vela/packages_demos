#ifndef PADDLE_H
#define PADDLE_H

#include "lvgl.h"
#include "breakout_types.h"

class Paddle {
public:
    // Constructor
    Paddle(lv_obj_t* parent, float startX, float startY, float width, float height, Rect screenBounds);
    
    // Destructor
    ~Paddle(); 

    // Member function declarations
    void update(float deltaTime);            // Update paddle position based on movement
    Rect getBoundingBox() const;             // Get the paddle's bounding rectangle
    void moveTo(float targetX);              // Move paddle to target X position
    void stopMovement();                      // Stop paddle movement
    float getX() const;                      // Get current X position
    float getY() const;                      // Get current Y position
    float getWidth() const;                  // Get current width
    void setWidth(float w);                  // Set paddle width

private:
    float m_x;                               // Current X position
    float m_y;                               // Current Y position
    float m_width;                           // Paddle width
    float m_height;                          // Paddle height
    float m_targetX;                          // Target X position for movement
    bool m_isMoving;                          // Whether paddle is moving
    lv_obj_t* m_parent;                       // Pointer to parent LVGL object
    Rect m_screenBounds;                      // Boundaries of the game screen
    lv_obj_t* m_gui_object;                   // Pointer to LVGL image object representing the paddle
};

#endif // PADDLE_H
