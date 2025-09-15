#ifndef POWERUP_H
#define POWERUP_H

#include "breakout_types.h"
#include <lvgl.h>

class PowerUp {
public:
    // Define available power-up types
    enum class Type {
        SPLIT_BALL,     // Split the ball into multiple balls
        EXTRA_PADDLE,   // Enlarge the paddle
        UP_BALLS,       //  Make all balls move upward
        TROPHY          // Level complete
    };

    PowerUp(lv_obj_t* parent, Type type, const Vec2& pos);
    ~PowerUp();

    void update(float deltaTime);   // Move downward
    void remove();                  // Destroy GUI object
    Type getType() const { return m_type; }
    Vec2 getPosition() const { return m_position; } // For collision
    bool isActive() const { return m_active; }

private:
    Type m_type;
    Vec2 m_position;
    float m_speed;      // Falling speed
    bool m_active;

    lv_obj_t* m_obj;    // LVGL object (image)
};

#endif // POWERUP_H