/**************************************
 * Paddle.cpp
 * Implementation file for the player-controlled paddle class
 * Responsible for paddle creation, updating, movement, and boundary checking logic.
 *************************************/
#include "Paddle.h" 
#include "GameResourceManager/GameResourceManager.h"
#include <algorithm>
#include <cmath>
#include <cstdio>



Paddle::Paddle(lv_obj_t* parent, float startX, float startY, float width, float height, Rect screenBounds)
    : m_x(startX),
      m_y(startY),
      m_width(width),
      m_height(height),
      m_targetX(0.0f),
      m_isMoving(false),
      m_parent(parent),
      m_screenBounds(screenBounds),
      m_gui_object(nullptr)
{
    //printf("DEBUG: Paddle constructor called.\n");

    // Image data and descriptor must remain static to ensure their lifetime

    GameResourceManager resourceManager;
    auto paddle_src = resourceManager.getIconSource("KUN.png"); 

    //printf("DEBUG: Paddle image requested from cache/file.\n");
    
    m_gui_object = lv_img_create(m_parent);
    if (!m_gui_object) {
        printf("Failed to create lv_img object\n");
        return;
    }
    
    if(paddle_src) {
        lv_img_set_src(m_gui_object, paddle_src); 
    }
    else{
        printf("Failed to load KUN.png\n");
    }
    lv_obj_set_size(m_gui_object, (lv_coord_t)m_width, (lv_coord_t)m_height);
    lv_obj_set_pos(m_gui_object, (lv_coord_t)m_x, (lv_coord_t)m_y);

    m_targetX = m_x + m_width / 2.0f;
}

Paddle::~Paddle() {
    printf("DEBUG: Paddle destructor called. Deleting lv_obj...\n");
    if (m_gui_object) {
        lv_obj_del(m_gui_object);
        m_gui_object = nullptr;
    }
}

void Paddle::update(float deltaTime) {
    if (m_isMoving) {
        float diff = m_targetX - (m_x + m_width / 2.0f);
        float speed = 950.0f; // Movement speed, can be adjusted as needed
        if (std::abs(diff) > 1.0f) {
            float move = speed * deltaTime;
            if (diff > 0) {
                m_x += std::min(move, diff);
            } else {
                m_x += std::max(-move, diff);
            }

            // Boundary check
            m_x = std::max(m_screenBounds.x, std::min(m_x, m_screenBounds.x + m_screenBounds.width - m_width));

            if (m_gui_object) {
                lv_obj_set_x(m_gui_object, (lv_coord_t)m_x);
            }
        } else {
            m_isMoving = false;
        }
    }
}

Rect Paddle::getBoundingBox() const {
    return {m_x, m_y, m_width, m_height};
}

void Paddle::moveTo(float targetX) {
    m_targetX = targetX;
    m_isMoving = true;
}

void Paddle::stopMovement() {
    m_isMoving = false;
}

float Paddle::getX() const { return m_x; }

float Paddle::getY() const { return m_y; }
float Paddle::getWidth() const { return m_width; }
void Paddle::setWidth(float w) {
    m_width = w;

    // Each KUN.png is 80 pixels wide
    const int baseWidth = 80.0f;
    // Ensure w is a multiple of 80
    int repeatCount = static_cast<int>(w / baseWidth);
    
    if (m_gui_object) {
        lv_obj_del(m_gui_object);
        m_gui_object = nullptr;
    }

    
    m_gui_object = lv_obj_create(m_parent);
    lv_obj_set_size(m_gui_object, (lv_coord_t)repeatCount * baseWidth, (lv_coord_t)m_height);
    lv_obj_set_pos(m_gui_object,(lv_coord_t)m_x, (lv_coord_t)m_y);
    lv_obj_clear_flag(m_gui_object, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(m_gui_object, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(m_gui_object, 0, 0);
    GameResourceManager resourceManager;
    const void* paddle_src = resourceManager.getIconSource("KUN.png");
    if (!paddle_src) {
        printf("Failed to load KUN.png\n");
        return;
    }

    for (int i = 0; i < repeatCount; ++i) {
        lv_obj_t* img = lv_img_create(m_gui_object); 
        lv_img_set_src(img, paddle_src);
        lv_obj_set_size(img, (lv_coord_t)baseWidth, (lv_coord_t)m_height);
        lv_obj_set_pos(img, (lv_coord_t)i * (baseWidth-10), (lv_coord_t)(-20)); 
    }
}