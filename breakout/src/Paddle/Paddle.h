#ifndef PADDLE_H
#define PADDLE_H

#include "lvgl.h"

#include "breakout_types.h"
class Paddle {
public:
    // 构造函数
    Paddle(lv_obj_t* parent, float startX, float startY, float width, float height, Rect screenBounds);
    
    // 析构函数
    ~Paddle(); 

    // 关键修复：添加之前缺失的所有成员函数的声明
    void update(float deltaTime);
    Rect getBoundingBox() const;
    void moveTo(float targetX);
    void stopMovement();
    float getX() const;
    float getY() const;

private:
    float m_x;
    float m_y;
    float m_width;
    float m_height;
    float m_targetX;
    bool m_isMoving;
    Rect m_screenBounds;
    lv_obj_t* m_gui_object; // 指向LVGL图片对象的指针
};

#endif // PADDLE_H
