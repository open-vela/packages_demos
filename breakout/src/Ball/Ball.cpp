#include "Ball.h"
#include "GameResourceManager/GameResourceManager.h"
#include <cmath>

// Define gravity
static const float GRAVITY = 350.0f;
// Define the maximum ball speed
static const float MAX_BALL_SPEED = 750.0f; 

Ball::Ball(lv_obj_t* parent, float radius) : m_state(State::HELD), m_radius(radius), m_gui_object(nullptr) {
    GameResourceManager resourceManager;
    auto ball_src = resourceManager.getIconSource("ball.png");

    printf("DEBUG: ball image loaded and decoded to RGB565 for the first time.\n");

    m_gui_object = lv_img_create(parent);
    if (!m_gui_object) {
        printf("Failed to create lv_img object\n");
        return;
    }
    if(ball_src) {
        lv_img_set_src(m_gui_object, ball_src); 
    }
    else{
        printf("Failed to load ball.png\n");
    }
    lv_obj_set_size(m_gui_object, m_radius * 2, m_radius * 2);
    lv_style_init(&m_style);
    lv_style_set_bg_color(&m_style, lv_color_white());
    lv_style_set_radius(&m_style, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&m_style, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_border_width(&m_style, 0);
    lv_obj_add_style(m_gui_object, &m_style, 0);
}

void Ball::update(float deltaTime) {
    if (m_state != State::MOVING) {
        return;
    }

    // Apply gravity
    m_vel.y += GRAVITY * deltaTime;

    // Calculate current speed squared
    float speed_sq = m_vel.x * m_vel.x + m_vel.y * m_vel.y;
    float max_speed_sq = MAX_BALL_SPEED * MAX_BALL_SPEED;

    // If current speed squared exceeds max speed squared
    if (speed_sq > max_speed_sq) {
        // Calculate actual current speed
        float current_speed = sqrtf(speed_sq);
        // Scale velocity vector proportionally to max speed
        m_vel.x = (m_vel.x / current_speed) * MAX_BALL_SPEED;
        m_vel.y = (m_vel.y / current_speed) * MAX_BALL_SPEED;
    }
    
    // Update position based on the finalized velocity
    m_pos.x += m_vel.x * deltaTime;
    m_pos.y += m_vel.y * deltaTime;
    
    // Update the LVGL object's position on screen
    lv_obj_set_pos(m_gui_object, (lv_coord_t)(m_pos.x - m_radius), (lv_coord_t)(m_pos.y - m_radius));
}

void Ball::stickToPaddle(const Rect& paddleRect) {
    if (m_state != State::HELD) return;
    m_pos.x = paddleRect.x + paddleRect.width / 2.0f;
    m_pos.y = paddleRect.y - m_radius;
    lv_obj_set_pos(m_gui_object, (lv_coord_t)(m_pos.x - m_radius), (lv_coord_t)(m_pos.y - m_radius));
}

void Ball::launch() { 
    if (m_state == State::HELD) { 
        m_state = State::MOVING; 
        m_vel = { 150.0f, -750.0f }; 
    } 
}

void Ball::bounceX() { 
    m_vel.x = -m_vel.x; 
}

void Ball::bounceY() {
    m_vel.y = -m_vel.y;
}

void Ball::boostSpeed(float factor) { 
    m_vel.x *= factor; 
    m_vel.y *= factor; 
}

void Ball::setPosition(const Vec2& pos) { 
    m_pos = pos; 
    lv_obj_set_pos(m_gui_object, (lv_coord_t)(m_pos.x - m_radius), (lv_coord_t)(m_pos.y - m_radius)); 
}

void Ball::setVelocity(const Vec2& vel) { 
    m_vel = vel; 
}

Ball::State Ball::getState() const { 
    return m_state; 
}

Vec2 Ball::getPosition() const { 
    return m_pos; 
}

Vec2 Ball::getVelocity() const { 
    return m_vel; 
}

float Ball::getRadius() const { 
    return m_radius; 
}