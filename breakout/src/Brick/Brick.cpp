/********************************************
 * Brick.cpp
 * Implementation file for the Brick class
 * Responsible for creating individual bricks, managing hit points, updating colors, and handling collisions.
 *******************************************/
#include "GameResourceManager/GameResourceManager.h"
#include "Brick.h" 

/*****************************************
 *  x      Brick's X coordinate
 *  y      Brick's Y coordinate
 *  width  Brick's width (also used for height as bricks are square)
 *  type   Character representing the brick type and its initial hit points
 ****************************************/
Brick::Brick(lv_obj_t* parent, float x, float y, float width, char type) 
    : m_is_active(true),
      m_rect{x, y, width, width},
      m_gui_object(nullptr)
{
    // Set hit points based on brick type character
    switch(type) {
        case '#': m_hp = -1; break;  // Indestructible wall
        case 'A': m_hp = 1; break;
        case 'B': m_hp = 2; break;
        case 'C': m_hp = 3; break;
        case 'D': m_hp = 4; break;
        case 'E': m_hp = 5; break;
        default:  m_hp = 1; break;
    }

    m_gui_object = lv_img_create(parent);
    if (!m_gui_object) {
        printf("Failed to create lv_img object\n");
        return;
    }
    updateImage();
    // Set position and size
    lv_obj_set_pos(m_gui_object, x, y);
    lv_obj_set_size(m_gui_object, width, width);

    // Add black border style
    lv_style_init(&m_border_style);
    lv_style_set_bg_color(&m_border_style, lv_color_white());
    lv_style_set_border_width(&m_border_style, 1);
    lv_style_set_border_color(&m_border_style, lv_color_black());
    lv_style_set_pad_all(&m_border_style, 0);
    lv_obj_add_style(m_gui_object, &m_border_style, 0);
}

/***************************************
 * Destructor automatically called when Brick object is deleted
 * Ensures corresponding UI object is also removed from screen to prevent memory leaks.
 ***************************************/
Brick::~Brick() { 
    if (m_gui_object) {
        lv_obj_del(m_gui_object); 
        m_gui_object = nullptr; 
    } 
    lv_style_reset(&m_border_style); 
}

/**
 * Called when the ball hits the brick
 */
void Brick::onHit() {
    // Only apply damage if hit points > 0 (i.e., not an indestructible wall)
    if (m_hp > 0) {
        m_hp--; // Decrease hit points by 1
        
        if (m_hp == 0) {
            // If hit points are depleted
            m_is_active = false; // Mark brick as inactive, collision checks will ignore it
            lv_obj_add_flag(m_gui_object, LV_OBJ_FLAG_HIDDEN); // Hide brick's UI object on screen
        } else {
            // If still has hit points, update color for visual feedback
            updateImage();
        }
    }
}

/**
 * Checks if the brick is currently active (not destroyed)
 * Returns true if active, false if destroyed
 */
bool Brick::isActive() const { 
    return m_is_active; 
}

/**
 * Gets the bounding box of the brick (collision box)
 * Returns a Rect containing position and size
 */
Rect Brick::getBoundingBox() const { 
    return m_rect; 
}

int Brick::getHP() const {
    return m_hp;
}

/**
 * Updates the brick's background image based on current hit points
 */
void Brick::updateImage() {
    if (!m_gui_object) return;

    std::string image_filename;
    switch (m_hp) {
        case -1: image_filename = "brick_#.png"; break;  // Wall: black
        case 1:  image_filename = "brick_A.png"; break;  // HP=1: green
        case 2:  image_filename = "brick_B.png"; break;  // HP=2: orange
        case 3:  image_filename = "brick_C.png"; break;  // HP=3: purple
        case 4:  image_filename = "brick_D.png"; break;  // HP=4: yellow
        case 5:  image_filename = "brick_E.png"; break;  // HP=5: blue
        default: image_filename = "brick_A.png"; break;  // green
    }

    GameResourceManager resourceManager;
    auto img_src = resourceManager.getIconSource(image_filename);
    if(img_src) {
        lv_img_set_src(m_gui_object, img_src); 
    }
    else{
        printf("Failed to load brick.png\n");
    }
    
}
