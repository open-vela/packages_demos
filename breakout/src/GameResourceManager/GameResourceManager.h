#ifndef GAME_RESOURCE_MANAGER_H
#define GAME_RESOURCE_MANAGER_H

#include <string>
#include <map>
#include "lvgl.h" 
#include "Audio/audio_ctl.h"

#define RES_ROOT CONFIG_LVX_BREAKOUT_DATA_ROOT

class Game;

class GameResourceManager {
public:
    // Constructor
    GameResourceManager();
    
    // Destructor
    ~GameResourceManager();

    // Resource path getters
    std::string getResPath();    // Get root resource path
    std::string getMapPath();    // Get map resource path
    std::string getFontsPath();  // Get fonts resource path
    std::string getIconsPath();  // Get icons resource path
    std::string getAudioPath();  // Get audio resource path

    // Audio control
    bool playAudio(int brickHp); // Play audio corresponding to brick HP
    void stopAudio();            // Stop audio playback

    // Map and background loading
    bool loadMap(const std::string& mapName, Game* game);   // Load a map file into the game
    void loadBackground(lv_obj_t* parent, const std::string& iconName); // Load background image

    // Image resource management
    const lv_draw_buf_t* getIconSource(const std::string& iconName); // Get cached or decoded image

    // Static level management
    static int getCurrentLevel();           // Get current game level
    static void setCurrentLevel(int level); // Set current game level
    
    // Static resource cache cleanup
    static void cleanupCache();             // Free all cached images

private:
    static std::map<std::string, lv_draw_buf_t*> s_imageCache; // Static image cache
    audioctl_s* m_audioCtl = nullptr;                          // Audio control handle
    static int currentLevel;                                   // Current game level
};

#endif // GAME_RESOURCE_MANAGER_H