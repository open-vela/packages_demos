#ifndef BALL_H
#define BALL_H

#pragma once

#include "breakout_types.h"



class Ball {
public:
    enum class State { HELD, MOVING, STATIC};
    Ball(lv_obj_t* parent, float radius,Ball::State);
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
    void setState(State state) { m_state = state;}
    lv_obj_t* getGuiObject() const;
    void setRadius(float r);

private:
    State m_state;
    Vec2 m_pos, m_vel;
    float m_radius;
    lv_obj_t* m_gui_object;
    lv_style_t m_style;
};

#endif // BALL_H