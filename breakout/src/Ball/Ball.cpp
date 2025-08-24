#include "Ball.h"
#include "GameResourceManager/GameResourceManager.h"
#include <cmath>

// Gravity acceleration
static const float GRAVITY = 300.0f;
// Maximum ball speed
static const float MAX_BALL_SPEED = 800.0f; 

// Constructor with optional initial state (default HELD)
Ball::Ball(lv_obj_t* parent, float radius, State initialState /* = HELD */)
    : m_state(initialState), m_radius(radius), m_gui_object(nullptr) 
{
    GameResourceManager resourceManager;
    auto ball_src = resourceManager.getIconSource("ball.png");

    // Create LVGL image object for the ball
    m_gui_object = lv_img_create(parent);
    if (!m_gui_object) {
        printf("Failed to create lv_img object\n");
        return;
    }
    if(ball_src) {
        lv_img_set_src(m_gui_object, ball_src); 
    } else {
        printf("Failed to load ball.png\n");
    }

    // Set size and style
    lv_obj_set_size(m_gui_object, m_radius * 2, m_radius * 2);
    lv_style_init(&m_style);
    lv_style_set_border_width(&m_style, 0);
    lv_obj_add_style(m_gui_object, &m_style, 0);

    // If the ball starts in MOVING state, give it an initial velocity
    if (m_state == State::MOVING) {
        m_vel = {150.0f, -800.0f}; 
    }
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

// Update ball position and velocity based on deltaTime
void Ball::update(float deltaTime) {
    if (m_state != State::MOVING) return;

    // Apply gravity
    m_vel.y += GRAVITY * deltaTime;

    // Limit speed for all balls (including those from pool)
    limitSpeed(MAX_BALL_SPEED);

    // Update position
    m_pos.x += m_vel.x * deltaTime;
    m_pos.y += m_vel.y * deltaTime;

    // Update LVGL GUI object position
    lv_obj_set_pos(m_gui_object, (lv_coord_t)(m_pos.x - m_radius), (lv_coord_t)(m_pos.y - m_radius));
}

// Stick the ball to the paddle when held
void Ball::stickToPaddle(const Rect& paddleRect) {
    if (m_state != State::HELD) return;
    m_pos.x = paddleRect.x + paddleRect.width / 2.0f;
    m_pos.y = paddleRect.y - m_radius;
    lv_obj_set_pos(m_gui_object, (lv_coord_t)(m_pos.x - m_radius), (lv_coord_t)(m_pos.y - m_radius));
}

// Launch the ball from HELD state
void Ball::launch() { 
    if (m_state == State::HELD) { 
        m_state = State::MOVING; 
        m_vel = {150.0f, -800.0f}; 
    } 
}

// Bounce the ball horizontally
void Ball::bounceX() { m_vel.x = -m_vel.x; }

// Bounce the ball vertically
void Ball::bounceY() { m_vel.y = -m_vel.y; }

// Boost the ball speed by a factor
void Ball::boostSpeed(float factor) { m_vel.x *= factor; m_vel.y *= factor; }

// Set ball position
void Ball::setPosition(const Vec2& pos) { 
    m_pos = pos; 
    lv_obj_set_pos(m_gui_object, (lv_coord_t)(m_pos.x - m_radius), (lv_coord_t)(m_pos.y - m_radius)); 
}

// Set ball velocity
void Ball::setVelocity(const Vec2& vel) { m_vel = vel; }

// Get current state
Ball::State Ball::getState() const { return m_state; }

// Get current position
Vec2 Ball::getPosition() const { return m_pos; }

// Get current velocity
Vec2 Ball::getVelocity() const { return m_vel; }

// Get ball radius
float Ball::getRadius() const { return m_radius; }

// Get LVGL GUI object
lv_obj_t* Ball::getGuiObject() const { return m_gui_object; }

// Set ball setRadius
void Ball::setRadius(float r) { m_radius = r; lv_obj_set_size(m_gui_object, r*2, r*2); }