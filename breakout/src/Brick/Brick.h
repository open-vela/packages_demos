#ifndef BRICK_H
#define BRICK_H

#pragma once
#include "breakout_types.h"

class Brick {
public:
    Brick(lv_obj_t* parent, float x, float y, float width, char type);
    ~Brick();
    void onHit();
    bool isActive() const;
    Rect getBoundingBox() const;
    int getHP() const;
    lv_obj_t* getGUIObject() const { return m_gui_object; }
private:
    void updateImage();
    int m_hp;
    bool m_is_active;
    Rect m_rect;
    lv_obj_t* m_gui_object;
    lv_style_t m_style;
    lv_style_t m_border_style;
};
#endif // BRICK_H