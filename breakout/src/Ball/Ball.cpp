#include "Ball.h"
#include "GameResourceManager/GameResourceManager.h"
#include <cmath>

// Gravity acceleration
static const float GRAVITY = 300.0f;
// Maximum ball speed
static const float MAX_BALL_SPEED = 900.0f;

// Constructor
Ball::Ball(lv_obj_t* parent, float radius, State initialState)
    : m_state(initialState), m_radius(radius), m_pos({0,0}), m_vel({0,0})
{
    GameResourceManager resourceManager;
    m_ball_src = resourceManager.getIconSource("ball.png");

}


void Ball::limitSpeed(float maxSpeed) {
    float speedSq = m_vel.x * m_vel.x + m_vel.y * m_vel.y;
    float maxSpeedSq = maxSpeed * maxSpeed;

    if (speedSq > maxSpeedSq) {
        float currentSpeed = sqrtf(speedSq);
        m_vel.x = (m_vel.x / currentSpeed) * maxSpeed;
        m_vel.y = (m_vel.y / currentSpeed) * maxSpeed;
    }
}

void Ball::update(float deltaTime) {
    if (m_state != State::MOVING) return;

    // Apply gravity
    m_vel.y += GRAVITY * deltaTime;

    // Limit speed
    limitSpeed(MAX_BALL_SPEED);

    // Update position
    m_pos.x += m_vel.x * deltaTime;
    m_pos.y += m_vel.y * deltaTime;
}

void Ball::stickToPaddle(const Rect& paddleRect) {
    if (m_state != State::HELD) return;
    m_pos.x = paddleRect.x + paddleRect.width / 2.0f;
    m_pos.y = paddleRect.y - m_radius;
}

void Ball::launch() {
    if (m_state == State::HELD) {
        m_state = State::MOVING;
        m_vel = {150.0f, -900.0f};
    }
}

void Ball::bounceX() { m_vel.x = -m_vel.x; }
void Ball::bounceY() { m_vel.y = -m_vel.y; }
void Ball::boostSpeed(float factor) { m_vel.x *= factor; m_vel.y *= factor; }

void Ball::setPosition(const Vec2& pos) { m_pos = pos; }
void Ball::setVelocity(const Vec2& vel) { m_vel = vel; }

Vec2 Ball::getPosition() const { return m_pos; }
Vec2 Ball::getVelocity() const { return m_vel; }
Ball::State Ball::getState() const { return m_state; }
float Ball::getRadius() const { return m_radius; }
void Ball::setState(State state) { m_state = state; }
void Ball::setRadius(float r) { m_radius = r; }
const lv_draw_buf_t* Ball::get_ball_src() {return m_ball_src;}

