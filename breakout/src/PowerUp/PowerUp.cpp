#include "PowerUp.h"
#include "GameResourceManager/GameResourceManager.h"
#include <cstdio>

PowerUp::PowerUp(lv_obj_t* parent, Type type, const Vec2& pos)
    : m_type(type), m_position(pos), m_speed(200.0f), m_active(true)
{
    m_obj = lv_img_create(parent);
    GameResourceManager resourceManager;

    switch (type) {
        case Type::SPLIT_BALL: {
            auto src = resourceManager.getIconSource("three_balls.png");
            lv_img_set_src(m_obj, src);
            break;
        }
        case Type::EXTRA_PADDLE: {
            auto src = resourceManager.getIconSource("friends.png");
            lv_img_set_src(m_obj, src);
            break;
        }
        case Type::UP_BALLS: {
            auto src = resourceManager.getIconSource("up_balls.png");
            lv_img_set_src(m_obj, src);
            break;
        }
        case Type::TROPHY: {
            auto src = resourceManager.getIconSource("trophy.png");
            lv_img_set_src(m_obj, src);
            break;
        }
    }

    lv_obj_set_pos(m_obj, (lv_coord_t)pos.x, (lv_coord_t)pos.y);
}

PowerUp::~PowerUp() {
    if (m_obj) {
        lv_obj_del(m_obj);
        m_obj = nullptr;
    }
}

void PowerUp::update(float deltaTime) {
    if (!m_active) return;
    m_position.y += m_speed * deltaTime; // fall down
    if (m_obj) {
        lv_obj_set_pos(m_obj, (lv_coord_t)m_position.x, (lv_coord_t)m_position.y);
    }
}

void PowerUp::remove() {
    m_active = false;
    if (m_obj) {
        lv_obj_del(m_obj);
        m_obj = nullptr;
    }
}
