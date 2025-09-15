#ifndef BALL_H
#define BALL_H

#pragma once

#include "breakout_types.h"

class Ball {
public:
    enum class State { HELD, MOVING, STATIC };

    Ball(lv_obj_t* parent, float radius, Ball::State state = State::HELD);

    void limitSpeed(float maxSpeed);
    void update(float deltaTime);
    void stickToPaddle(const Rect& paddleRect);
    void launch();
    void bounceX();
    void bounceY();
    void setPosition(const Vec2& pos);
    void setVelocity(const Vec2& vel);
    void boostSpeed(float factor);

    Vec2 getVelocity() const;
    State getState() const;
    Vec2 getPosition() const;
    float getRadius() const;
    void setState(State state);
    void setRadius(float r);
    const lv_draw_buf_t* get_ball_src();
    

private:
    State m_state;
    float m_radius;
    Vec2 m_pos;
    Vec2 m_vel;
    const lv_draw_buf_t* m_ball_src;
    uint8_t* m_buf;            
};

#endif // BALL_H